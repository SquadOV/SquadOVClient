export interface Bookmark {
    matchUuid: string
    userId: number
    username: string
    tm: Date
    label: string
    icon: string
    eventId: number
}

export function cleanBookmarkFromJson(b: Bookmark): Bookmark {
    b.tm = new Date(b.tm)
    return b
}