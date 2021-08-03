import { openUrlInBrowser} from '@client/js/external'
import { apiClient, ApiData} from '@client/js/api'

export function redirectToTwitchLogin(): Promise<string> {
    return new Promise((resolve, reject) => {
        apiClient.getTwitchOauthAuthorizeUrl().then((resp: ApiData<string>) => {
            openUrlInBrowser(resp.data)
            resolve(resp.data)
        }).catch((err: any) => {
            reject(err)
        })
    })
}