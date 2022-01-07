#pragma once

#include <atomic>
#include <memory>
#include <optional>
#include <shared_mutex>

#include <aws/core/Aws.h>
#include <aws/identity-management/auth/CognitoCachingCredentialsProvider.h>
#include <aws/identity-management/auth/PersistentCognitoIdentityProvider.h>

#include "shared/json.h"

namespace service::api {

struct AwsCognitoCredentials {
    std::string token;
    std::string idp;
    std::string identityId;
    std::string poolId;
    std::string region;
    std::string accountId;
};

class ManualCognitoIdentityProvider: public Aws::Auth::PersistentCognitoIdentityProvider {
public:
    void loadFromCognitoCredentials(const AwsCognitoCredentials& cred);

    bool HasIdentityId() const override;
    bool HasLogins() const override;

    Aws::String GetIdentityId() const override;
    Aws::Map<Aws::String, Aws::Auth::LoginAccessTokens> GetLogins() override;
    Aws::String GetAccountId() const override;
    Aws::String GetIdentityPoolId() const override;

    void PersistIdentityId(const Aws::String& id) override;
    void PersistLogins(const Aws::Map<Aws::String, Aws::Auth::LoginAccessTokens>& logins) override;
private:
    mutable std::shared_mutex _mutex;
    Aws::String _accountId;
    std::optional<Aws::String> _identityId;
    Aws::String _identityPoolId;
    Aws::Map<Aws::String, Aws::Auth::LoginAccessTokens> _logins;
};

class AwsAuthenticatedApi {
public:
    AwsAuthenticatedApi();
    ~AwsAuthenticatedApi();

private:
    void onSessionUpdated(bool checkInProgress = true);
    int64_t _sessionUpdateCbId = 0;

    std::shared_mutex _credentialMutex;
    Aws::SDKOptions _options;
    std::shared_ptr<ManualCognitoIdentityProvider> _identityProvider;
    std::shared_ptr<Aws::Auth::CognitoCachingAuthenticatedCredentialsProvider> _credentialProvider;
    std::atomic<int64_t> _retryCount = 0;
    std::atomic<bool> _updateInProgress = false;
};

AwsAuthenticatedApi* getAwsApi();

using AwsAuthenticatedApiPtr = std::unique_ptr<AwsAuthenticatedApi>;

}

namespace shared::json {

template<>
struct JsonConverter<service::api::AwsCognitoCredentials> {
    static service::api::AwsCognitoCredentials from(const nlohmann::json& v) {
        service::api::AwsCognitoCredentials cred;
        cred.token = JsonConverter<decltype(cred.token)>::from(v["token"]);
        cred.idp = JsonConverter<decltype(cred.idp)>::from(v["idp"]);
        cred.identityId = JsonConverter<decltype(cred.identityId)>::from(v["identityId"]);
        cred.poolId = JsonConverter<decltype(cred.poolId)>::from(v["poolId"]);
        cred.region = JsonConverter<decltype(cred.poolId)>::from(v["region"]);
        cred.accountId = JsonConverter<decltype(cred.poolId)>::from(v["accountId"]);
        return cred;
    }
};

}