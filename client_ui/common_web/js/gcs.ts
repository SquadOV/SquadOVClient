import axios from 'axios'

export async function uploadLocalFileToGcs(localFile: string, gcsUri: string) {
    let res = await axios.post(gcsUri, {}, {
        headers: {
            'x-goog-resumable': 'start',
            'content-type': 'application/octet-stream',
        }
    })
    if (res.status != 201) {
        throw(`Failed to post to initial resumable GCS upload: ${res.data}`)
    }

    let putUri = res.headers['Location']
    let putRes = await axios.put(putUri, {}, {
        headers: {
            'Content-Length': '0',
            'Content-Range': '',
        }
    })    
    return putUri
}