<template>
    <div>
        <h1>HELLO WORLD</h1>
    </div>
</template>

<script>

import { apiClient } from '@client/js/api.js'

export default {
    data: () => ({
        allAccounts: []        
    }),

    mounted() {
        apiClient.listValorantAccounts().then((resp) => {
            this.allAccounts = resp.data
        })
    }
}

</script>