export function routeLevelToKey(route: any, level: number): string {
    console.log(route.matched.length, level)
    if (route.matched.length >= (level + 1)) {
        let data = route.matched[level]
        let regex = new RegExp(data.regex.source.substr(0, data.regex.source.length - 1), data.regex.flags)

        let match = route.fullPath.match(regex)
        if (!!match) {
            return match[0]
        } else {
            return route.fullPath
        }
    } else {
        return route.fullPath
    }
}