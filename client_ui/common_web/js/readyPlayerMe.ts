import axios, { AxiosResponse } from "axios"

export async function createReadyPlayerMeRender(
    url: String,
    scene: String = "fullbody-portrait-v1",
    armature: String = "ArmatureTargetMale",
    mouthSmile: number = 0.2,
    browInnerUp: number = 0,
    // For list of other settings, see https://docs.readyplayer.me/render-api/render-api
): Promise<string> {
    return axios
        .post("https://render.readyplayer.me/render", {
            model: url,
            scene: scene,
            armature: armature,
            blendShapes: {
                Wolf3D_Head: {
                    mouthSmile: mouthSmile,
                    browInnerUp: browInnerUp,
                },
            },
        })
        .then((resp: AxiosResponse) => {
            return resp.data.renders[0]
        })
}
