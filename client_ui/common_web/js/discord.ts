import { openUrlInBrowser} from '@client/js/external'
import { apiClient, ApiData} from '@client/js/api'

export function redirectToDiscordLogin(): Promise<string> {
    return new Promise((resolve, reject) => {
        apiClient.getDiscordOauthAuthorizeUrl().then((resp: ApiData<string>) => {
            openUrlInBrowser(resp.data)
            resolve(resp.data)
        }).catch((err: any) => {
            reject(err)
        })
    })
}