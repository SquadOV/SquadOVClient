var global_gtm

export default function({ $gtm, route }) {
    global_gtm = $gtm
}

export function initGtm() {
    console.log('init gtm')
    global_gtm.init('GTM-NMS74ND')
}