import { openUrlInBrowser} from '@client/js/external'
import { apiClient, ApiData} from '@client/js/api'

export function redirectToRsoLogin(userId: number): Promise<void> {
    return new Promise((resolve, reject) => {
        apiClient.getRiotRsoAuthorizeUrl(userId).then((resp: ApiData<string>) => {
            openUrlInBrowser(resp.data)
            resolve()
        }).catch((err: any) => {
            reject(err)
        })
    })
}