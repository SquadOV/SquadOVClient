import axios from 'axios'

const waitForApiServerSetup = (target, key, descriptor) => {
    const apiFn = async function(...args) {
        let port = process.env.SQUADOV_API_PORT
        let key = process.env.SQUADOV_API_KEY

        while (port === undefined || key === undefined) {
            port = process.env.SQUADOV_API_PORT
            key = process.env.SQUADOV_API_KEY
            await new Promise(resolve => setTimeout(resolve, 100))
        }

        const original = descriptor.value
        return original.apply(this, ...args)
    }
    return {
        ...descriptor,
        value: apiFn,
        initializer: undefined
    }
}

class ApiClient {
    createAxiosConfig(endpoint) {
        return {
            url: `http://127.0.0.1:${process.env.SQUADOV_API_PORT}/${endpoint}`,
            headers: {
                'Authorization': `Bearer ${process.env.SQUADOV_API_KEY}`
            }
        }
    }

    @waitForApiServerSetup
    listValorantAccounts() {
        const cfg = this.createAxiosConfig('valorant/accounts')
        
        return axios({
            ...cfg,
            method: 'get'
        })
    }
}

export let apiClient = new ApiClient()