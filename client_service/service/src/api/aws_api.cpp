#include "api/aws_api.h"
#include "api/squadov_api.h"
#include "shared/errors/error.h"
#include "shared/log/log.h"

#include <aws/core/utils/logging/ConsoleLogSystem.h>
#include <aws/core/utils/logging/FormattedLogSystem.h>
#include <aws/core/http/windows/WinHttpSyncHttpClient.h>
#include <aws/auth/credentials.h>
#include <aws/cognito-identity/model/GetCredentialsForIdentityRequest.h>

#include <chrono>

namespace service::api {
namespace {

class AwsLoggerInterop: public Aws::Utils::Logging::FormattedLogSystem {
public:
    AwsLoggerInterop();

    void Flush() override;
private:
    void ProcessFormattedStatement(Aws::String&& statement) override;
};

AwsLoggerInterop::AwsLoggerInterop():
    Aws::Utils::Logging::FormattedLogSystem(
#ifdef NDEBUG
        Aws::Utils::Logging::LogLevel::Info
#else
        Aws::Utils::Logging::LogLevel::Trace
#endif
    )
{
}

void AwsLoggerInterop::Flush() {
    LOG_FLUSH();
}

void AwsLoggerInterop::ProcessFormattedStatement(Aws::String&& statement) {
    LOG_INFO("[AWS] " << statement);
}

}

void ManualCognitoIdentityProvider::loadFromCognitoCredentials(const AwsCognitoCredentials& cred) {
    PersistIdentityId(cred.identityId);

    Aws::Map<Aws::String, Aws::Auth::LoginAccessTokens> newLogins;

    Aws::Auth::LoginAccessTokens tok;
    tok.accessToken = cred.token;

    newLogins["cognito-identity.amazonaws.com"] = tok;
    PersistLogins(newLogins);

    {
        std::lock_guard guard(_mutex);
        _accountId = cred.accountId;
        _identityPoolId = cred.poolId;
    }
}

bool ManualCognitoIdentityProvider::HasIdentityId() const {
    std::shared_lock guard(_mutex);
    return _identityId.has_value();
}

bool ManualCognitoIdentityProvider::HasLogins() const {
    std::shared_lock guard(_mutex);
    return !_logins.empty();
}

Aws::String ManualCognitoIdentityProvider::GetIdentityId() const {
    std::shared_lock guard(_mutex);
    return _identityId.value();
}

Aws::Map<Aws::String, Aws::Auth::LoginAccessTokens> ManualCognitoIdentityProvider::GetLogins() {
    std::shared_lock guard(_mutex);
    return _logins;
}

Aws::String ManualCognitoIdentityProvider::GetAccountId() const {
    std::shared_lock guard(_mutex);
    return _accountId;
}

Aws::String ManualCognitoIdentityProvider::GetIdentityPoolId() const {
    std::shared_lock guard(_mutex);
    return _identityPoolId;
}

void ManualCognitoIdentityProvider::PersistIdentityId(const Aws::String& id) {
    std::lock_guard guard(_mutex);
    _identityId = id;
    
    if (m_identityIdUpdatedCallback) {
        m_identityIdUpdatedCallback(*this);
    }
}

void ManualCognitoIdentityProvider::PersistLogins(const Aws::Map<Aws::String, Aws::Auth::LoginAccessTokens>& logins) {
    std::lock_guard guard(_mutex);
    _logins = logins;

    if (m_loginsUpdatedCallback) {
        m_loginsUpdatedCallback(*this);
    }
}

AwsAuthenticatedApi* getAwsApi() {
    static auto singleton = std::make_unique<AwsAuthenticatedApi>();
    Aws::Utils::DateTime::SetProvider(std::make_shared<AwsTimeProvider>());
    return singleton.get();
}

AwsAuthenticatedApi::AwsAuthenticatedApi() {
    _options.loggingOptions.logLevel = 
#ifdef NDEBUG
        Aws::Utils::Logging::LogLevel::Info
#else
        Aws::Utils::Logging::LogLevel::Trace
#endif
    ;
    _options.loggingOptions.logger_create_fn = [](){
        return std::make_shared<AwsLoggerInterop>();
    };
    Aws::InitAPI(_options);

    onSessionUpdated();
    _sessionUpdateCbId = service::api::getGlobalApi()->addSessionIdUpdateCallback([this](const std::string&){
        onSessionUpdated();
    });
}

AwsAuthenticatedApi::~AwsAuthenticatedApi() {
    service::api::getGlobalApi()->removeSessionIdUpdateCallback(_sessionUpdateCbId);
    Aws::ShutdownAPI(_options);
}

void AwsAuthenticatedApi::onSessionUpdated() {
    std::lock_guard guard(_credentialMutex);
    const auto credentials = service::api::getGlobalApi()->getAwsCredentials();
    _lastCredentials = credentials;

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::String(credentials.region);

    // The credentials that we get from our server is an OpenID token.
    // We need to store this data into our PersistentCognitoIdentityProvider which is given to a
    // CognitoCachingAuthenticatedCredentialsProvider. We can then use the CognitoCachingAuthenticatedCredentialsProvider
    // object as our credential provider for future AWS clients.
    if (!_identityProvider) {
        _identityProvider = std::make_shared<ManualCognitoIdentityProvider>();
    }
    _identityProvider->loadFromCognitoCredentials(credentials);

    if (!_credentialProvider) {
        _credentialProvider = std::make_shared<Aws::Auth::CognitoCachingAuthenticatedCredentialsProvider>(
            _identityProvider,
            std::make_shared<Aws::CognitoIdentity::CognitoIdentityClient>(clientConfig)
        );
    }

    const auto creds = _credentialProvider->GetAWSCredentials();
    LOG_INFO("Refresh AWS Credentials: " << creds.GetAWSAccessKeyId() << std::endl);
}

std::shared_ptr<Aws::Http::HttpClient> AwsAuthenticatedApi::createHttpClient() const {
    std::shared_lock guard(_credentialMutex);

    Aws::Client::ClientConfiguration clientConfig;
    clientConfig.region = Aws::String(_lastCredentials.region);
    clientConfig.requestTimeoutMs = 10000;
    return std::make_shared<Aws::Http::WinHttpSyncHttpClient>(clientConfig);
}

std::shared_ptr<Aws::Client::AWSAuthV4Signer> AwsAuthenticatedApi::createSigner(const std::string& serviceName) const {
    std::shared_lock guard(_credentialMutex);
    return std::make_shared<Aws::Client::AWSAuthV4Signer>(
        _credentialProvider,
        serviceName.c_str(),
        Aws::String(_lastCredentials.region)
    );
}

}