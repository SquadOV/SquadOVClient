const { Pool, Client } = require('pg')
const fs = require('fs')
const bent = require('bent')

function gameToString(input) {
    let game
    switch (input) {
        case 0:
            game = 'Aim Lab'
            break
        case 1:
            game = 'Hearthstone'
            break
        case 2:
            game = 'League of Legends'
            break
        case 3:
            game = 'Teamfight Tactics'
            break
        case 4:
            game = 'Valorant'
            break
        case 5:
            game = 'World of Warcraft'
            break
        case 6:
            game = 'CSGO'
            break
    }
    return game
}

class ApiServer {
    constructor(config) {
        this.pool = new Pool({
            user: config.database.username,
            password: config.database.password,
            host: config.database.host,
            database: config.database.db,
            port: config.database.port,
            ssl: !!config.database.ssl ? {
                rejectUnauthorized: false,
                ca: fs.readFileSync(config.database.ssl.ca).toString()
            }: false
        })
    }

    async getAverageAgeKpi(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
            WITH series(tm) AS (
                SELECT *
                FROM generate_series(
                    DATE_TRUNC('day', $1::TIMESTAMPTZ),
                    DATE_TRUNC('day', $2::TIMESTAMPTZ),
                    INTERVAL '1 ${pgInterval}'
                )
            )
            SELECT
                s.tm AS "tm",
                COALESCE((
                    SELECT PERCENTILE_CONT(0.5) WITHIN GROUP (ORDER BY inp.v)
                    FROM (
                        SELECT
                            inp.v,
                            percent_rank() OVER (ORDER BY inp.v DESC) AS "pct"
                        FROM (
                            SELECT DISTINCT ON (u.id)
                                (EXTRACT(EPOCH FROM (s.tm + INTERVAL '1 week')) - EXTRACT(EPOCH FROM u.registration_time)) / 60 / 60 / 24 AS "v"
                            FROM squadov.daily_active_endpoint AS dae
                            INNER JOIN squadov.users AS u
                                ON u.id = dae.user_id
                            WHERE dae.tm >= s.tm AND dae.tm < (s.tm + INTERVAL '1 week')
                                AND u.registration_time IS NOT NULL
                        ) AS inp
                    ) AS inp
                    WHERE inp.pct <= 0.5
                ), 0) AS "val"
            FROM series AS s
            `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return rows.map((r) => {
            return {
                tm: r.tm,
                data: {
                    'Value': parseInt(r.val),
                },
                sub: ['Value']
            }
        })
    }

    async getActiveUsers(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
            WITH series(tm) AS (
                SELECT *
                FROM generate_series(
                    DATE_TRUNC('day', $1::TIMESTAMPTZ),
                    DATE_TRUNC('day', $2::TIMESTAMPTZ),
                    INTERVAL '1 ${pgInterval}'
                )
            )
            SELECT
                s.tm AS "tm",
                COALESCE((
                    SELECT COUNT(DISTINCT user_id)
                    FROM squadov.daily_active_sessions AS das
                    WHERE das.tm >= s.tm AND das.tm < s.tm + INTERVAL '1 ${pgInterval}'
                ), 0) AS "session",
                COALESCE((
                    SELECT COUNT(DISTINCT user_id)
                    FROM squadov.daily_active_endpoint AS dae
                    WHERE dae.tm >= s.tm AND dae.tm < (s.tm + INTERVAL '1 ${pgInterval}')
                ), 0) AS "endpoint",
                COALESCE((
                    SELECT COUNT(DISTINCT v.user_uuid)
                    FROM squadov.vods AS v
                    WHERE v.start_time IS NOT NULL AND v.user_uuid IS NOT NULL
                        AND v.start_time >= s.tm AND v.start_time < s.tm + INTERVAL '1 ${pgInterval}'
                ), 0) AS "vod",
                COALESCE((
                    SELECT COUNT(DISTINCT v.user_uuid)
                    FROM squadov.vods AS v
                    INNER JOIN squadov.users AS u
                        ON u.uuid = v.user_uuid
                    LEFT JOIN squadov.daily_active_endpoint AS dae
                        ON dae.user_id = u.id
                            AND dae.tm >= s.tm AND dae.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    WHERE v.start_time IS NOT NULL AND v.user_uuid IS NOT NULL
                        AND v.start_time >= s.tm AND v.start_time < s.tm + INTERVAL '1 ${pgInterval}'
                        AND dae.user_id IS NULL
                ), 0) AS "passive"
            FROM series AS s
            `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return rows.map((r) => {
            return {
                tm: r.tm,
                data: {
                    'Session': parseInt(r.session),
                    'Endpoint': parseInt(r.endpoint),
                    'VOD': parseInt(r.vod),
                    'Passive': parseInt(r.passive),
                },
                sub: ['Session', 'Endpoint', 'VOD', 'Passive']
            }
        })
    }

    async getNewlyRegisteredUsers(interval, start, end, referralCodes) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            g.code AS "code",
            g.cnt AS "count",
            g.ver AS "verified"
        FROM series AS s
        CROSS JOIN LATERAL (
            SELECT
                rc.code,
                COUNT(DISTINCT u.id),
                COUNT(DISTINCT (CASE WHEN u.verified THEN u.id END))
            FROM squadov.users AS u
            LEFT JOIN squadov.user_referral_code_usage AS rcu
                ON rcu.email = u.email
            LEFT JOIN squadov.referral_codes AS rc
                ON rc.id = rcu.code_id
            WHERE u.registration_time IS NOT NULL AND u.registration_time >= s.tm AND u.registration_time < s.tm + INTERVAL '1 ${pgInterval}'
            GROUP BY rc.code
        ) AS g(code, cnt, ver)
            `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        let retData = new Map()

        for (let r of rows) {
            let tmKey = new Date(r.tm).getTime()
            if (!retData.has(tmKey)) {
                let d = {
                    tm: r.tm,
                    data: {
                        'Users': 0,
                        'Organic': 0,
                        'Verified': 0,
                    },
                    sub: ['Users', 'Organic', 'Verified', ...referralCodes]
                }

                for (let r of referralCodes) {
                    d.data[r] = 0
                }
                retData.set(tmKey, d)
            }
            
            let d = retData.get(tmKey)
            let cnt = parseInt(r.count)
            let ver = parseInt(r.verified)
            d.data['Verified'] = ver
            if (!r.code) {
                d.data['Organic'] = cnt
            } else {
                if (r.code in d.data) {
                    d.data[r.code] = cnt
                }
            }

            d.data['Users'] += cnt
        }

        return Array.from(retData.values()).sort((a, b) => {
            return new Date(a.tm).getTime() - new Date(b.tm).getTime()
        })
    }

    async getLostUsers(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
            WITH series(tm) AS (
                SELECT *
                FROM generate_series(
                    DATE_TRUNC('day', $1::TIMESTAMPTZ),
                    DATE_TRUNC('day', $2::TIMESTAMPTZ),
                    INTERVAL '1 ${pgInterval}'
                )
            )
            SELECT
                s.tm AS "tm",
                COALESCE((
                    SELECT COUNT(DISTINCT u1.user_id)
                    FROM (
                        SELECT user_id
                        FROM squadov.daily_active_sessions AS das
                        WHERE das.tm >= s.tm - INTERVAL '1 ${pgInterval}' AND das.tm < s.tm
                    ) AS u1
                    LEFT JOIN (
                        SELECT user_id
                        FROM squadov.daily_active_sessions AS das
                        WHERE das.tm >= s.tm AND das.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    ) AS u2
                        ON u1.user_id = u2.user_id
                    WHERE u2.user_id IS NULL
                ), 0) AS "users"
            FROM series AS s
            `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return rows.map((r) => {
            return {
                tm: r.tm,
                data: {
                    'Users': parseInt(r.users),
                },
                sub: ['Users']
            }
        })
    }

    async getChurn(interval, start, end) {
        // For each interval, we need to know the number of users
        // at the beginning of the period, the number of users
        // at the end of the period, and the number of users that
        // were new in each time interval.
        let activeUsers = await this.getActiveUsers(interval, start, end)
        let registeredUsers = await this.getNewlyRegisteredUsers(interval, start, end, [])
        let lostUsers = await this.getLostUsers(interval, start, end)

        let ret = []
        for (let i = 1; i < activeUsers.length; ++i) {
            ret.push({
                tm: activeUsers[i].tm,
                data: {
                    'Churn': lostUsers[i].data['Users'] / (activeUsers[i-1].data['Session'] + registeredUsers[i].data['Users'])
                },
                sub: ['Churn']
            })
        }
        return ret
    }

    async getReferralUsersMetric(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.referral_visits AS rd
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rd.code
                WHERE rd.tm >= s.tm AND rd.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    AND rc.user_id IS NOT NULL
            ), 0) AS "visits",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.referral_downloads AS rd
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rd.code
                WHERE rd.tm >= s.tm AND rd.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    AND rc.user_id IS NOT NULL
            ), 0) AS "downloads",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.user_referral_code_usage AS ucu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = ucu.code_id
                WHERE ucu.tm >= s.tm AND ucu.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    AND rc.user_id IS NOT NULL
            ), 0) AS "registrations"
        FROM series AS s
            `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return rows.map((r) => {
            return {
                tm: r.tm,
                data: {
                    'Visits': parseInt(r.visits),
                    'Downloads': parseInt(r.downloads),
                    'Registrations': parseInt(r.registrations),
                },
                sub: ['Visits', 'Downloads', 'Registrations']
            }
        })
    }

    async getReferralUsersBreakdown() {
        const query = `
        SELECT
            rc.code AS "code",
            u.username AS "description",
            v.value AS "visits",
            d.value AS "downloads",
            r.value AS "registrations"
        FROM squadov.referral_codes AS rc
        INNER JOIN squadov.users AS u
            ON u.id = rc.user_id
        CROSS JOIN LATERAL (
            SELECT COUNT(1) AS "value"
            FROM squadov.referral_visits AS rd
            WHERE rd.code = rc.id
        ) AS v
        CROSS JOIN LATERAL (
            SELECT COUNT(1) AS "value" 
            FROM squadov.referral_downloads AS rd
            WHERE rd.code = rc.id
        ) AS d
        CROSS JOIN LATERAL (
            SELECT COUNT(1) AS "value"
            FROM squadov.user_referral_code_usage AS ucu
            WHERE ucu.code_id = rc.id
        ) AS r
        WHERE rc.user_id IS NOT NULL
        `

        const { rows } = await this.pool.query(
            query,
        )

        return rows.map((r) => {
            return {
                code: r.code,
                description: r.description,
                visits: parseInt(r.visits),
                downloads: parseInt(r.downloads),
                registrations: parseInt(r.registrations),
            }
        })
    }

    async getReferralCampaignsMetric(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.referral_visits AS rd
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rd.code
                WHERE rd.tm >= s.tm AND rd.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    AND rc.user_id IS NULL
            ), 0) AS "visits",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.referral_downloads AS rd
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rd.code
                WHERE rd.tm >= s.tm AND rd.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    AND rc.user_id IS NULL
            ), 0) AS "downloads",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.user_referral_code_usage AS ucu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = ucu.code_id
                WHERE ucu.tm >= s.tm AND ucu.tm < s.tm + INTERVAL '1 ${pgInterval}'
                    AND rc.user_id IS NULL
            ), 0) AS "registrations"
        FROM series AS s
            `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return rows.map((r) => {
            return {
                tm: r.tm,
                data: {
                    'Visits': parseInt(r.visits),
                    'Downloads': parseInt(r.downloads),
                    'Registrations': parseInt(r.registrations),
                },
                sub: ['Visits', 'Downloads', 'Registrations']
            }
        })
    }

    async getReferralCampaignsBreakdown() {
        const query = `
        SELECT
            rc.code AS "code",
            rc.description AS "description",
            v.value AS "visits",
            d.value AS "downloads"
        FROM squadov.referral_codes AS rc
        CROSS JOIN LATERAL (
            SELECT COUNT(1) AS "value"
            FROM squadov.referral_visits AS rd
            WHERE rd.code = rc.id
        ) AS v
        CROSS JOIN LATERAL (
            SELECT COUNT(1) AS "value"
            FROM squadov.referral_downloads AS rd
            WHERE rd.code = rc.id
        ) AS d
        CROSS JOIN LATERAL (
            SELECT COUNT(1) AS "value"
            FROM squadov.user_referral_code_usage AS ucu
            WHERE ucu.code_id = rc.id
        ) AS r
        WHERE rc.user_id IS NULL
        `

        const { rows } = await this.pool.query(
            query
        )

        return rows.map((r) => {
            return {
                code: r.code,
                description: r.description,
                visits: parseInt(r.visits),
                downloads: parseInt(r.downloads),
                registrations: parseInt(r.registrations),
            }
        })
    }

    async getGames(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            g.game AS "game",
            g.cnt AS "count"
        FROM series AS s
        CROSS JOIN LATERAL (
            SELECT m.game, COUNT(DISTINCT m.uuid)
            FROM squadov.vods AS v
            INNER JOIN squadov.matches AS m
                ON m.uuid = v.match_uuid
            WHERE v.is_clip = FALSE
                AND v.end_time IS NOT NULL
                AND v.start_time >= s.tm AND v.end_time < s.tm + INTERVAL '1 ${pgInterval}'
            GROUP BY m.game
        ) AS g(game, cnt)
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        // Each row has 'tm', 'game', 'count'.
        // To return from the API, we want to group all the games in a single row for each instance of 'tm'.
        let retData = new Map()

        for (let r of rows) {
            let tmKey = new Date(r.tm).getTime()
            if (!retData.has(tmKey)) {
                retData.set(tmKey, {
                    tm: r.tm,
                    data: {
                        'Total': 0,
                        'Aim Lab': 0,
                        'CS:GO': 0,
                        'Hearthstone': 0,
                        'LoL': 0,
                        'TFT': 0,
                        'Valorant': 0,
                        'WoW': 0,
                    },
                    sub: ['Total', 'Aim Lab', 'CS:GO', 'Hearthstone', 'LoL', 'TFT', 'Valorant', 'WoW']
                })
            }
            
            let d = retData.get(tmKey)
            let cnt = parseInt(r.count)
            switch (parseInt(r.game)) {
                case 0:
                    d.data['Aim Lab'] = cnt
                    break
                case 1:
                    d.data['Hearthstone'] = cnt
                    break
                case 2:
                    d.data['LoL'] = cnt
                    break
                case 3:
                    d.data['TFT'] = cnt
                    break
                case 4:
                    d.data['Valorant'] = cnt
                    break
                case 5:
                    d.data['WoW'] = cnt
                    break
                case 6:
                    d.data['CS:GO'] = cnt
                    break
            }

            d.data['Total'] += cnt
        }

        return Array.from(retData.values()).sort((a, b) => {
            return new Date(a.tm).getTime() - new Date(b.tm).getTime()
        })
    }

    async getSquadInvites(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.squad_membership_invites AS m
                WHERE m.invite_time >= s.tm AND m.invite_time < s.tm + INTERVAL '1 ${pgInterval}'
            ), 0) AS "sent",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.squad_membership_invites AS m
                WHERE m.response_time >= s.tm AND m.response_time < s.tm + INTERVAL '1 ${pgInterval}'
                    AND m.joined
            ), 0) AS "accepted",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.squad_membership_invites AS m
                WHERE m.response_time >= s.tm AND m.response_time < s.tm + INTERVAL '1 ${pgInterval}'
                    AND NOT m.joined
            ), 0) AS "rejected"
        FROM series AS s
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return rows.map((r) => {
            return {
                tm: r.tm,
                data: {
                    'Sent': parseInt(r.sent),
                    'Accepted': parseInt(r.accepted),
                    'Rejected': parseInt(r.rejected),
                },
                sub: ['Sent', 'Accepted', 'Rejected']
            }
        })
    }

    async getSquads(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.squads AS m
                WHERE m.creation_time >= s.tm AND m.creation_time < s.tm + INTERVAL '1 ${pgInterval}'
                    AND NOT m.is_default
            ), 0) AS "squads"
        FROM series AS s
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return rows.map((r) => {
            return {
                tm: r.tm,
                data: {
                    'Squads': parseInt(r.squads),
                },
                sub: ['Squads']
            }
        })
    }

    async getVods(interval, start, end, useTimeHours, clips) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            g.game AS "game",
            g.cnt AS "count"
        FROM series AS s
        CROSS JOIN LATERAL (
            SELECT m.game, ${ useTimeHours ? 'SUM(EXTRACT(EPOCH FROM v.end_time) - EXTRACT(EPOCH FROM v.start_time))::FLOAT / 3600.0' : 'COUNT(DISTINCT v.video_uuid)'}
            FROM squadov.vods AS v
            INNER JOIN squadov.matches AS m
                ON m.uuid = v.match_uuid
            WHERE v.is_clip = $3
                AND v.end_time IS NOT NULL
                AND v.start_time >= s.tm AND v.end_time < s.tm + INTERVAL '1 ${pgInterval}'
            GROUP BY m.game
        ) AS g(game, cnt)
        `

        const { rows } = await this.pool.query(
            query,
            [start, end, clips]
        )

        // Each row has 'tm', 'game', 'count'.
        // To return from the API, we want to group all the games in a single row for each instance of 'tm'.
        let retData = new Map()

        for (let r of rows) {
            let tmKey = new Date(r.tm).getTime()
            if (!retData.has(tmKey)) {
                retData.set(tmKey, {
                    tm: r.tm,
                    data: {
                        'Total': 0,
                        'Aim Lab': 0,
                        'CS:GO': 0,
                        'Hearthstone': 0,
                        'LoL': 0,
                        'TFT': 0,
                        'Valorant': 0,
                        'WoW': 0,
                    },
                    sub: ['Total', 'Aim Lab', 'CS:GO', 'Hearthstone', 'LoL', 'TFT', 'Valorant', 'WoW']
                })
            }
            
            let d = retData.get(tmKey)
            let cnt = useTimeHours ? parseFloat(parseFloat(r.count).toFixed(2)) : parseInt(r.count)
            switch (parseInt(r.game)) {
                case 0:
                    d.data['Aim Lab'] = cnt
                    break
                case 1:
                    d.data['Hearthstone'] = cnt
                    break
                case 2:
                    d.data['LoL'] = cnt
                    break
                case 3:
                    d.data['TFT'] = cnt
                    break
                case 4:
                    d.data['Valorant'] = cnt
                    break
                case 5:
                    d.data['WoW'] = cnt
                    break
                case 6:
                    d.data['CS:GO'] = cnt
                    break
            }

            d.data['Total'] += cnt
        }

        return Array.from(retData.values()).sort((a, b) => {
            return new Date(a.tm).getTime() - new Date(b.tm).getTime()
        })
    }

    async getWatchTime(interval, start, end) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            g.game AS "game",
            g.hours AS "hours"
        FROM series AS s
        CROSS JOIN LATERAL (
            SELECT m.game, SUM(vwa.end_seconds - vwa.start_seconds)::FLOAT / 3600.0
            FROM squadov.vod_watch_analytics AS vwa
            INNER JOIN squadov.vods AS v
                ON v.video_uuid = vwa.video_uuid
            INNER JOIN squadov.matches AS m
                ON m.uuid = v.match_uuid
            WHERE NOT v.is_clip
                AND v.end_time IS NOT NULL
                AND vwa.tm >= s.tm AND vwa.tm < s.tm + INTERVAL '1 ${pgInterval}'
            GROUP BY m.game
        ) AS g(game, hours)
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        // Each row has 'tm', 'game', 'count'.
        // To return from the API, we want to group all the games in a single row for each instance of 'tm'.
        let retData = new Map()

        for (let r of rows) {
            let tmKey = new Date(r.tm).getTime()
            if (!retData.has(tmKey)) {
                retData.set(tmKey, {
                    tm: r.tm,
                    data: {
                        'Total': 0,
                        'Aim Lab': 0,
                        'CS:GO': 0,
                        'Hearthstone': 0,
                        'LoL': 0,
                        'TFT': 0,
                        'Valorant': 0,
                        'WoW': 0,
                    },
                    sub: ['Total', 'Aim Lab', 'CS:GO', 'Hearthstone', 'LoL', 'TFT', 'Valorant', 'WoW']
                })
            }
            
            let d = retData.get(tmKey)
            let hours = parseFloat(parseFloat(r.hours).toFixed(2))
            switch (parseInt(r.game)) {
                case 0:
                    d.data['Aim Lab'] = hours
                    break
                case 1:
                    d.data['Hearthstone'] = hours
                    break
                case 2:
                    d.data['LoL'] = hours
                    break
                case 3:
                    d.data['TFT'] = hours
                    break
                case 4:
                    d.data['Valorant'] = hours
                    break
                case 5:
                    d.data['WoW'] = hours
                    break
                case 6:
                    d.data['CS:GO'] = hours
                    break
            }

            d.data['Total'] += hours
        }

        return Array.from(retData.values()).sort((a, b) => {
            return new Date(a.tm).getTime() - new Date(b.tm).getTime()
        })
    }

    async getViralCoefficient() {
        let data = (await this.pool.query(
            `
            SELECT (
                SELECT COUNT(1)
                FROM squadov.users
            ) AS "users",
            (
                SELECT COUNT(1)
                FROM squadov.squad_membership_invites
            ) AS "sentInvites",
            (
                SELECT COUNT(1)
                FROM squadov.squad_membership_invites
                WHERE response_time IS NOT NULL
                    AND joined
            ) AS "acceptedInvites",
            (
                SELECT COUNT(1)
                FROM squadov.referral_codes AS rc
                INNER JOIN squadov.referral_visits AS rv
                    ON rv.code = rc.id
                WHERE rc.user_id IS NOT NULL
            ) AS "referralVisits",
            (
                SELECT COUNT(1)
                FROM squadov.referral_codes AS rc
                INNER JOIN squadov.referral_downloads AS rv
                    ON rv.code = rc.id
                WHERE rc.user_id IS NOT NULL
            ) AS "referralDownloads"
            `            
        )).rows[0]

        let totalUsers = parseInt(data.users)
        let sentInvites = parseInt(data.sentInvites)
        let acceptedInvites = parseInt(data.acceptedInvites)
        let referralVisits = parseInt(data.referralVisits)
        let referralDownloads = parseInt(data.referralDownloads)

        let invitesPerUser = (sentInvites + referralVisits) / totalUsers
        let conversionRate = (acceptedInvites + referralDownloads) / (sentInvites + referralVisits)
        return invitesPerUser * conversionRate
    }
    
    async getKpiUsers() {
        return parseInt( (await this.pool.query(
            `
            SELECT COUNT(1) AS "kpi"
            FROM squadov.users
            `            
        )).rows[0].kpi)
    }

    async getKpiVods() {
        return parseInt( (await this.pool.query(
            `
            SELECT COUNT(1) AS "kpi"
            FROM squadov.vods
            WHERE match_uuid IS NOT NULL
            `            
        )).rows[0].kpi)
    }

    async getKpiMatches() {
        return parseInt( (await this.pool.query(
            `
            SELECT COUNT(1) AS "kpi"
            FROM squadov.matches
            `            
        )).rows[0].kpi)
    }

    async getKpi(kpi) {
        switch (kpi) {
            case 0:
                return await this.getViralCoefficient()
            case 1:
                return await this.getKpiUsers()
            case 2:
                return await this.getKpiVods()
            case 3:
                return await this.getKpiMatches()
        }
    }

    async getBreakdown(breakdown, start, end, extra) {
        switch (breakdown) {
            case 0:
                return await this.getRegistrationBreakdown(start, end)
            case 1:
                return await this.getWatchTimeGameBreakdown(start, end)
            case 2:
                return await this.getWatchTimeUserBreakdown(start, end)
            case 3:
                return await this.getActiveUserGameBreakdown(start, end)

        }
    }

    async getMetrics(metric, interval, start, end, extra) {
        switch (metric) {
            case 0:
                return await this.getActiveUsers(interval, start, end)
            case 1:
                return await this.getChurn(interval, start, end)
            case 2:
                return await this.getReferralUsersMetric(interval, start, end)
            case 3:
                return await this.getReferralCampaignsMetric(interval, start, end)
            case 4: // Games
                return await this.getGames(interval, start, end)
            case 5: // Registrations
                return await this.getNewlyRegisteredUsers(interval, start, end, !!extra.referralCodes ? JSON.parse(extra.referralCodes) : [])
            case 6: // Squad Invites
                return await this.getSquadInvites(interval, start, end)
            case 7: // Squads
                return await this.getSquads(interval, start, end)
            case 8: // VODs
                return await this.getVods(interval, start, end, extra.useTimeHours === 'true', false)
            case 9: // Lost Users
                return await this.getLostUsers(interval, start, end)
            case 10: // Average Age
                return await this.getAverageAgeKpi(interval, start, end)
            case 11: // Clips
                return await this.getVods(interval, start, end, extra.useTimeHours === 'true', true)
            case 12: // Watch time
                return await this.getWatchTime(interval, start, end)
        }
    }

    async getCohortSize(interval, start, end, onlyActive) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        const query = `
        WITH series(tm) AS (
            SELECT *
            FROM generate_series(
                DATE_TRUNC('day', $1::TIMESTAMPTZ),
                DATE_TRUNC('day', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "cohort_key",
            COUNT(DISTINCT u.id) AS "count"
        FROM series AS s
        LEFT JOIN squadov.users AS u
            ON u.registration_time >= s.tm AND u.registration_time < s.tm + INTERVAL '1 ${pgInterval}'
        ${
            onlyActive ? 'INNER JOIN squadov.daily_active_endpoint AS dae ON dae.user_id = u.id' : ''
        }
        GROUP BY cohort_key
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        return new Map(rows.map((r) => {
            return [r.cohort_key.toISOString(), r.count]
        }))
    }

    async getInstalledUserCohorts(interval, start, end, period, length) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        let pgPeriod
        if (period == 0) {
            pgPeriod = 'day'
        } else if (period == 1) {
            pgPeriod = 'week'
        } else { 
            pgPeriod = 'month'
        }

        const query = `
            SELECT
                DATE_TRUNC('${pgInterval}', u.registration_time) AS "cohort_key",
                coh.cohort_period,
                SUM(1) AS "count"
            FROM squadov.users AS u
            CROSS JOIN LATERAL (
                SELECT DISTINCT ON (gs.tm) gs.tm, (EXTRACT(EPOCH FROM gs.tm - u.registration_time) / EXTRACT(EPOCH FROM INTERVAL '1 ${pgPeriod}'))::INTEGER
                FROM generate_series(
                    u.registration_time,
                    u.registration_time + INTERVAL '${length} ${pgPeriod}',
                    INTERVAL '1 ${pgPeriod}'
                ) AS gs(tm)
                INNER JOIN squadov.daily_active_sessions AS das
                    ON das.tm >= DATE_TRUNC('day', gs.tm) AND das.tm < DATE_TRUNC('day', gs.tm) + INTERVAL '${length} ${pgPeriod}'
                        AND das.user_id = u.id
            ) AS coh(tm, cohort_period)
            WHERE u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ) + INTERVAL '1 ${pgInterval}'
            GROUP BY cohort_key, coh.cohort_period
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        let cohortSizes = await this.getCohortSize(interval, start, end, false)
        let cohortData = new Map()
        for (let [key, size] of cohortSizes) {
            cohortData.set(key, {
                tm: key,
                data: new Array(length).fill(0),
                size: size,
            })
        }

        rows.forEach((r) => {
            let key = r.cohort_key.toISOString()
            let obj = cohortData.get(key)
            if (!obj) {
                return
            }

            if (r.cohort_period >= 0 && r.cohort_period < length) {
                obj.data[r.cohort_period] = r.count
            }

            cohortData.set(key, obj)
        })

        return Array.from(cohortData.values()).sort((a, b) => {
            return new Date(a.tm).getTime() - new Date(b.tm).getTime()
        })
    }

    async getActiveUserCohorts(interval, start, end, period, length) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        let pgPeriod
        if (period == 0) {
            pgPeriod = 'day'
        } else if (period == 1) {
            pgPeriod = 'week'
        } else { 
            pgPeriod = 'month'
        }

        const query = `
            SELECT
                DATE_TRUNC('${pgInterval}', u.registration_time) AS "cohort_key",
                coh.cohort_period,
                SUM(1) AS "count"
            FROM squadov.users AS u
            CROSS JOIN LATERAL (
                SELECT DISTINCT ON (gs.tm) gs.tm, (EXTRACT(EPOCH FROM gs.tm - u.registration_time) / EXTRACT(EPOCH FROM INTERVAL '1 ${pgPeriod}'))::INTEGER
                FROM generate_series(
                    u.registration_time,
                    u.registration_time + INTERVAL '${length} ${pgPeriod}',
                    INTERVAL '1 ${pgPeriod}'
                ) AS gs(tm)
                INNER JOIN squadov.daily_active_endpoint AS das
                    ON das.tm >= DATE_TRUNC('day', gs.tm) AND das.tm < DATE_TRUNC('day', gs.tm) + INTERVAL '${length} ${pgPeriod}'
                        AND das.user_id = u.id
            ) AS coh(tm, cohort_period)
            WHERE u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ) + INTERVAL '1 ${pgInterval}'
            GROUP BY cohort_key, coh.cohort_period
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        let cohortSizes = await this.getCohortSize(interval, start, end, true)
        let cohortData = new Map()
        for (let [key, size] of cohortSizes) {
            cohortData.set(key, {
                tm: key,
                data: new Array(length).fill(0),
                size: size,
            })
        }

        rows.forEach((r) => {
            let key = r.cohort_key.toISOString()
            let obj = cohortData.get(key)
            if (!obj) {
                return
            }

            if (r.cohort_period >= 0 && r.cohort_period < length) {
                obj.data[r.cohort_period] = r.count
            }

            cohortData.set(key, obj)
        })

        return Array.from(cohortData.values()).sort((a, b) => {
            return new Date(a.tm).getTime() - new Date(b.tm).getTime()
        })
    }

    async getRecordingUserCohorts(interval, start, end, period, length) {
        let pgInterval
        if (interval == 0) {
            pgInterval = 'day'
        } else if (interval == 1) {
            pgInterval = 'week'
        } else { 
            pgInterval = 'month'
        }

        let pgPeriod
        if (period == 0) {
            pgPeriod = 'day'
        } else if (period == 1) {
            pgPeriod = 'week'
        } else { 
            pgPeriod = 'month'
        }

        const query = `
            SELECT
                DATE_TRUNC('${pgInterval}', u.registration_time) AS "cohort_key",
                coh.cohort_period,
                SUM(1) AS "count"
            FROM squadov.users AS u
            CROSS JOIN LATERAL (
                SELECT DISTINCT ON (gs.tm) gs.tm, (EXTRACT(EPOCH FROM gs.tm - u.registration_time) / EXTRACT(EPOCH FROM INTERVAL '1 ${pgPeriod}'))::INTEGER
                FROM generate_series(
                    u.registration_time,
                    u.registration_time + INTERVAL '${length} ${pgPeriod}',
                    INTERVAL '1 ${pgPeriod}'
                ) AS gs(tm)
                INNER JOIN squadov.vods AS v
                    ON v.start_time >= DATE_TRUNC('day', gs.tm) AND v.start_time < DATE_TRUNC('day', gs.tm) + INTERVAL '${length} ${pgPeriod}'
                        AND v.user_uuid = u.uuid
                WHERE v.match_uuid IS NOT NULL
            ) AS coh(tm, cohort_period)
            WHERE u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ) + INTERVAL '1 ${pgInterval}'
            GROUP BY cohort_key, coh.cohort_period
        `

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        let cohortSizes = await this.getCohortSize(interval, start, end, true)
        let cohortData = new Map()
        for (let [key, size] of cohortSizes) {
            cohortData.set(key, {
                tm: key,
                data: new Array(length).fill(0),
                size: size,
            })
        }

        rows.forEach((r) => {
            let key = r.cohort_key.toISOString()
            let obj = cohortData.get(key)
            if (!obj) {
                return
            }

            if (r.cohort_period >= 0 && r.cohort_period < length) {
                obj.data[r.cohort_period] = r.count
            }

            cohortData.set(key, obj)
        })

        return Array.from(cohortData.values()).sort((a, b) => {
            return new Date(a.tm).getTime() - new Date(b.tm).getTime()
        })
    }

    async getCohort(cohort, interval, start, end, period, length) {
        switch (cohort) {
            case 0:
                return await this.getInstalledUserCohorts(interval, start, end, period, length)
            case 1:
                return await this.getActiveUserCohorts(interval, start, end, period, length)
            case 2:
                return await this.getRecordingUserCohorts(interval, start, end, period, length)
        }
    }

    async getReferralBreakdown(referral) {
        switch (referral) {
            case 0:
                return await this.getReferralCampaignsBreakdown()
            case 1:
                return await this.getReferralUsersBreakdown()
        }
    }

    async getAvailableReferrals() {
        const query = `
            SELECT code
            FROM squadov.referral_codes
        `

        const { rows } = await this.pool.query(
            query,
        )

        return rows.map((ele) => ele.code)
    }

    async getFunnelData(start, end, codes, organicOnly, verifiedOnly) {
        let query
        let params

        if (codes.length > 0) {
            query = `
            SELECT (
                SELECT COUNT(rv.code)
                FROM squadov.referral_visits AS rv
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rv.code
                WHERE rc.code = ANY($3)
                    AND rv.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND rv.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
            ) AS "view", (
                SELECT COUNT(DISTINCT rcu.email)
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND rcu.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
            ) AS "reg", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                INNER JOIN squadov.users AS u
                    ON u.email = rcu.email
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND rcu.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "login", (
                SELECT COUNT(DISTINCT ud.user_id)
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                INNER JOIN squadov.users AS u
                    ON u.email = rcu.email
                INNER JOIN squadov.user_downloads AS ud
                    ON ud.user_id = u.id
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND rcu.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "download", (
                SELECT COUNT(DISTINCT uhs.user_id)
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                INNER JOIN squadov.users AS u
                    ON u.email = rcu.email
                INNER JOIN squadov.user_hardware_specs AS uhs
                    ON uhs.user_id = u.id
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND rcu.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "install", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                INNER JOIN squadov.users AS u
                    ON u.email = rcu.email
                INNER JOIN squadov.vods AS v
                    ON v.user_uuid = u.uuid
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND rcu.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND v.is_clip = FALSE
                    AND v.end_time IS NOT NULL
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "record", (
                SELECT COUNT(DISTINCT u.id) AS "count"
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                INNER JOIN squadov.users AS u
                    ON u.email = rcu.email
                INNER JOIN squadov.daily_active_endpoint AS dae
                    ON dae.user_id = u.id
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND rcu.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "active"
            `

            params = [start, end, codes, verifiedOnly]
        } else {
            query = `SELECT (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.users AS u
                LEFT JOIN squadov.user_referral_code_usage AS rcu
                    ON rcu.email = u.email
                WHERE (NOT $3 OR rcu.code_id IS NULL)
                    AND u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "view", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.users AS u
                LEFT JOIN squadov.user_referral_code_usage AS rcu
                    ON rcu.email = u.email
                WHERE (NOT $3 OR rcu.code_id IS NULL)
                    AND u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "reg", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.users AS u
                LEFT JOIN squadov.user_referral_code_usage AS rcu
                    ON rcu.email = u.email
                WHERE (NOT $3 OR rcu.code_id IS NULL)
                    AND u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "login", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.users AS u
                INNER JOIN squadov.user_hardware_specs AS uhs
                    ON uhs.user_id = u.id
                LEFT JOIN squadov.user_referral_code_usage AS rcu
                    ON rcu.email = u.email
                WHERE (NOT $3 OR rcu.code_id IS NULL)
                    AND u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "install", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.users AS u
                INNER JOIN squadov.vods AS v
                    ON v.user_uuid = u.uuid
                LEFT JOIN squadov.user_referral_code_usage AS rcu
                    ON rcu.email = u.email
                WHERE (NOT $3 OR rcu.code_id IS NULL)
                    AND u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND v.is_clip = FALSE
                    AND v.end_time IS NOT NULL
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "record", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.users AS u
                INNER JOIN squadov.daily_active_endpoint AS dae
                    ON dae.user_id = u.id
                LEFT JOIN squadov.user_referral_code_usage AS rcu
                    ON rcu.email = u.email
                WHERE (NOT $3 OR rcu.code_id IS NULL)
                    AND u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                    AND (NOT $4::BOOLEAN OR u.verified)
            ) AS "active"`
            params = [start, end, organicOnly, verifiedOnly]
        }

        const { rows } = await this.pool.query(
            query,
            params,
        )

        return rows[0]
    }

    async createCampaign(code, desc) {
        await this.pool.query(
            `
            INSERT INTO squadov.referral_codes (
                id,
                code,
                description,
                user_id,
                tm
            ) VALUES (
                gen_random_uuid(),
                $1,
                $2,
                NULL,
                NOW()
            )
            `,
            [code, desc]
        )
    }

    async searchForUser(search) {
        let userCondition =  isNaN(search) ? `
            u.username = $1 OR u.email = $1 OR u.uuid::VARCHAR = $1
        ` : `
            u.id = $1
        `

        const { rows } = await this.pool.query(
            `
            SELECT
                u.id,
                u.username,
                u.email,
                u.verified,
                u.registration_time AS "registrationTime",
                MAX(dae.tm) AS "lastActive",
                MAX(us.issue_tm) AS "lastSession",
                MAX(v.end_time) AS "lastVod"
            FROM squadov.users AS u
            LEFT JOIN squadov.user_sessions AS us
                ON us.user_id = u.id
            LEFT JOIN squadov.daily_active_endpoint AS dae
                ON dae.user_id = u.id
            LEFT JOIN squadov.vods AS v
                ON v.user_uuid = u.uuid
            WHERE ${userCondition}
            GROUP BY u.id, u.username, u.email, u.verified, u.registration_time
            `,
            [isNaN(search) ? search : parseInt(search)]
        )

        if (rows.length > 0) {
            return {
                accounts: {
                    linkedRiot: (await this.pool.query(
                        `
                        SELECT DISTINCT CONCAT(ra.game_name, '#', ra.tag_line) AS "name"
                        FROM squadov.riot_account_links AS ral
                        INNER JOIN squadov.riot_accounts AS ra
                            ON ra.puuid = ral.puuid
                        WHERE ral.user_id = $1
                        `,
                        [rows[0].id]
                    )).rows.map((ele) => ele.name),
                    linkedTwitch: (await this.pool.query(
                        `
                        SELECT DISTINCT ta.twitch_name AS "name"
                        FROM squadov.linked_twitch_accounts AS lta
                        INNER JOIN squadov.twitch_accounts AS ta
                            ON ta.twitch_user_id = lta.twitch_user_id
                        WHERE lta.user_id = $1
                        `,
                        [rows[0].id]
                    )).rows.map((ele) => ele.name),
                    wowChars: (await this.pool.query(
                        `
                        SELECT DISTINCT wucc.unit_name AS "name"
                        FROM squadov.wow_user_character_cache AS wucc
                        WHERE wucc.user_id = $1
                        `,
                        [rows[0].id]
                    )).rows.map((ele) => ele.name),
                    steamAccounts: (await this.pool.query(
                        `
                        SELECT DISTINCT suc.steam_name AS "name"
                        FROM squadov.steam_user_links AS sul
                        INNER JOIN squadov.steam_users_cache AS suc
                            ON suc.steam_id = sul.steam_id
                        WHERE sul.user_id = $1
                        `,
                        [rows[0].id]
                    )).rows.map((ele) => ele.name)
                },
                hw: await (async (id) => {
                    const { rows } = await this.pool.query(
                        `
                        SELECT
                            CONCAT(os->>'name', ' ', os->>'majorVersion', ' ', os->>'minorVersion', ' ', os->>'edition') AS "os",
                            CONCAT(cpu->>'brand', ' ', cpu->>'clock', 'MHz ', cpu->>'cores', ' Cores') AS "cpu",
                            ram_kb AS "ramKb",
                            display#>>'{gpus, 0, name}' AS "gpu0",
                            display#>>'{gpus, 1, name}' AS "gpu1"
                        FROM squadov.user_hardware_specs
                        WHERE user_id = $1
                        `,
                        [id]
                    )

                    if (rows.length > 0) {
                        return rows[0]
                    } else {
                        return null
                    }
                })(rows[0].id),
                ...rows[0]
            }
        } else {
            return null
        }
    }

    async searchForVod(video, user) {
        let condition =  video !== '' ? `
            v.video_uuid = $1::UUID
        ` : `
            u.id = $1
        `

        const { rows } = await this.pool.query(
            `
            SELECT DISTINCT ON (v.video_uuid, v.start_time)
                v.video_uuid AS "videoUuid",
                v.user_uuid AS "userUuid",
                v.match_uuid AS "matchUuid",
                v.is_clip AS "isClip",
                v.is_local AS "isLocal",
                v.start_time AS "startTime",
                v.end_time AS "endTime",
                vm.has_fastify AS "isProcessed",
                vm.res_x AS "resX",
                vm.res_y AS "resY",
                vm.fps AS "fps",
                st.friendly_name AS "shareToken"
            FROM squadov.vods AS v
            INNER JOIN squadov.users AS u
                ON u.uuid = v.user_uuid
            LEFT JOIN squadov.vod_metadata AS vm
                ON vm.video_uuid = v.video_uuid
            LEFT JOIN squadov.share_tokens AS st
                ON st.match_uuid = v.match_uuid OR st.clip_uuid = v.video_uuid
            WHERE ${condition}
            ORDER BY v.start_time DESC, v.video_uuid
            LIMIT 20
            `,
            [(video !== '') ? video : parseInt(user)]
        )

        for (let r of rows) {
            r.oembed = null

            if (!!r.shareToken) {
                let req = bent('GET', 'json', 200)
                let url = new URL(`https://api.squadov.gg/oembed`)
                url.searchParams.append('url', `https://app.squadov.gg/share/${r.shareToken}`)
                url.searchParams.append('format', 'json')
                r.oembed = await req(url.href)
            }
        }

        return rows
    }

    async searchAimLab(params) {
        let conditions = []
        let bind = []

        if (params.match.trim().length > 0) {
            conditions.push(`m.uuid = $1`)
            bind.push(params.match.trim())
        }

        if (!isNaN(params.user)) {
            conditions.push(`at.user_id = $${conditions.length+1}`)
            bind.push(parseInt(params.user))
        }

        const { rows } = await this.pool.query(
            `
            SELECT
                m.uuid AS "matchUuid",
                at.create_date AS "time",
                at.task_name AS "task",
                at.score AS "score",
                u.uuid AS "userUuid",
                v.video_uuid AS "videoUuid"
            FROM squadov.matches AS m
            INNER JOIN squadov.aimlab_tasks AS at
                ON at.match_uuid = m.uuid
            INNER JOIN squadov.users AS u
                ON u.id = at.user_id
            LEFT JOIN squadov.vods AS v
                ON v.match_uuid = m.uuid
                    AND v.user_uuid = u.uuid
                    AND v.is_clip = FALSE
            WHERE ${conditions.join(' AND ')}
            ORDER BY at.create_date DESC
            LIMIT 20
            `,
            bind
        )

        return rows
    }

    async searchCsgo(params) {
        let conditions = []
        let bind = []

        if (params.match.trim().length > 0) {
            conditions.push(`m.uuid = $1`)
            bind.push(params.match.trim())
        }

        if (!isNaN(params.user)) {
            conditions.push(`cmv.user_id = $${conditions.length+1}`)
            bind.push(parseInt(params.user))
        }

        const { rows } = await this.pool.query(
            `
            SELECT
                m.uuid AS "matchUuid",
                u.uuid AS "userUuid",
                v.video_uuid AS "videoUuid",
                cmv.start_time AS "time",
                cmv.map AS "map",
                cmv.mode AS "mode",
                cmv.has_demo AS "hasDemo"
            FROM squadov.matches AS m
            INNER JOIN squadov.csgo_match_views AS cmv
                ON cmv.match_uuid = m.uuid
            INNER JOIN squadov.users AS u
                ON u.id = cmv.user_id
            LEFT JOIN squadov.vods AS v
                ON v.match_uuid = m.uuid
                    AND v.user_uuid = u.uuid
                    AND v.is_clip = FALSE
            WHERE ${conditions.join(' AND ')}
            ORDER BY cmv.start_time DESC
            LIMIT 20
            `,
            bind
        )

        return rows
    }

    async searchHearthstone(params) {
        let conditions = []
        let bind = []

        if (params.match.trim().length > 0) {
            conditions.push(`m.uuid = $1`)
            bind.push(params.match.trim())
        }

        if (!isNaN(params.user)) {
            conditions.push(`hmv.user_id = $${conditions.length+1}`)
            bind.push(parseInt(params.user))
        }

        const { rows } = await this.pool.query(
            `
            SELECT
                m.uuid AS "matchUuid",
                u.uuid AS "userUuid",
                v.video_uuid AS "videoUuid",
                hm.match_time AS "time"
            FROM squadov.matches AS m
            INNER JOIN squadov.hearthstone_matches AS hm
                ON hm.match_uuid = m.uuid
            INNER JOIN squadov.hearthstone_match_view AS hmv
                ON hmv.match_uuid = m.uuid
            INNER JOIN squadov.users AS u
                ON u.id = hmv.user_id
            LEFT JOIN squadov.vods AS v
                ON v.match_uuid = m.uuid
                    AND v.user_uuid = u.uuid
                    AND v.is_clip = FALSE
            WHERE ${conditions.join(' AND ')}
            ORDER BY hm.match_time DESC
            LIMIT 20
            `,
            bind
        )

        return rows
    }

    async searchLol(params) {
        let conditions = []
        let bind = []

        if (params.match.trim().length > 0) {
            conditions.push(`m.uuid = $1`)
            bind.push(params.match.trim())
        }

        if (!isNaN(params.user)) {
            conditions.push(`ral.user_id = $${conditions.length+1}`)
            bind.push(parseInt(params.user))
        }

        const { rows } = await this.pool.query(
            `
            SELECT
                m.uuid AS "matchUuid",
                u.uuid AS "userUuid",
                v.video_uuid AS "videoUuid",
                lm.game_creation AS "time",
                lm.game_id AS "riotId"
            FROM squadov.matches AS m
            INNER JOIN squadov.lol_match_info AS lm
                ON lm.match_uuid = m.uuid
            LEFT JOIN squadov.lol_match_participant_identities AS lmpi
                ON lmpi.match_uuid = lm.match_uuid
            LEFT JOIN squadov.riot_accounts AS ra
                ON ra.summoner_id = lmpi.summoner_id
            LEFT JOIN squadov.riot_account_links AS ral
                ON ral.puuid = ra.puuid
            INNER JOIN squadov.users AS u
                ON u.id = ral.user_id
            LEFT JOIN squadov.vods AS v
                ON v.match_uuid = m.uuid
                    AND v.user_uuid = u.uuid
                    AND v.is_clip = FALSE
            WHERE ${conditions.join(' AND ')}
            ORDER BY lm.game_creation DESC
            LIMIT 20
            `,
            bind
        )

        return rows
    }

    async searchTft(params) {
        let conditions = []
        let bind = []

        if (params.match.trim().length > 0) {
            conditions.push(`m.uuid = $1`)
            bind.push(params.match.trim())
        }

        if (!isNaN(params.user)) {
            conditions.push(`ral.user_id = $${conditions.length+1}`)
            bind.push(parseInt(params.user))
        }

        const { rows } = await this.pool.query(
            `
            SELECT
                m.uuid AS "matchUuid",
                u.uuid AS "userUuid",
                v.video_uuid AS "videoUuid",
                tmi.game_datetime AS "time",
                tm.match_id AS "riotId"
            FROM squadov.matches AS m
            INNER JOIN squadov.tft_matches AS tm
                ON tm.match_uuid = m.uuid
            INNER JOIN squadov.tft_match_info AS tmi
                ON tmi.match_uuid = tm.match_uuid
            LEFT JOIN squadov.tft_match_participants AS tmp
                ON tmp.match_uuid = tm.match_uuid
            LEFT JOIN squadov.riot_account_links AS ral
                ON ral.puuid = tmp.puuid
            INNER JOIN squadov.users AS u
                ON u.id = ral.user_id
            LEFT JOIN squadov.vods AS v
                ON v.match_uuid = m.uuid
                    AND v.user_uuid = u.uuid
                    AND v.is_clip = FALSE
            WHERE ${conditions.join(' AND ')}
            ORDER BY tmi.game_datetime DESC
            LIMIT 20
            `,
            bind
        )

        return rows
    }

    async searchValorant(params) {
        let conditions = []
        let bind = []

        if (params.match.trim().length > 0) {
            conditions.push(`m.uuid = $1`)
            bind.push(params.match.trim())
        }

        if (!isNaN(params.user)) {
            conditions.push(`ral.user_id = $${conditions.length+1}`)
            bind.push(parseInt(params.user))
        }

        const { rows } = await this.pool.query(
            `
            SELECT
                m.uuid AS "matchUuid",
                u.uuid AS "userUuid",
                v.video_uuid AS "videoUuid",
                vm.server_start_time_utc AS "time",
                vmul.match_id AS "riotId",
                vm.map_id AS "map",
                vm.game_mode AS "mode",
                vm.is_ranked AS "isRanked"
            FROM squadov.matches AS m
            INNER JOIN squadov.valorant_matches AS vm
                ON vm.match_uuid = m.uuid
            INNER JOIN squadov.valorant_match_uuid_link AS vmul
                ON vmul.match_uuid = vm.match_uuid
            LEFT JOIN squadov.valorant_match_players AS vmp
                ON vmp.match_uuid = vmul.match_uuid
            LEFT JOIN squadov.riot_account_links AS ral
                ON ral.puuid = vmp.puuid
            INNER JOIN squadov.users AS u
                ON u.id = ral.user_id
            LEFT JOIN squadov.vods AS v
                ON v.match_uuid = m.uuid
                    AND v.user_uuid = u.uuid
                    AND v.is_clip = FALSE
            WHERE ${conditions.join(' AND ')}
            ORDER BY vm.server_start_time_utc DESC
            LIMIT 20
            `,
            bind
        )

        return rows
    }

    async searchWow(params) {
        let conditions = []
        let bind = []

        if (params.match.trim().length > 0) {
            conditions.push(`m.uuid = $1`)
            bind.push(params.match.trim())
        }

        if (!isNaN(params.user)) {
            conditions.push(`wmv.user_id = $${conditions.length+1}`)
            bind.push(parseInt(params.user))
        }

        const { rows } = await this.pool.query(
            `
            SELECT
                m.uuid AS "matchUuid",
                u.uuid AS "userUuid",
                v.video_uuid AS "videoUuid",
                wmv.start_tm AS "time",
                wmv.build_version AS "gameVersion",
                wav.arena_type AS "arenaType",
                wev.encounter_name AS "encounterName",
                wev.difficulty AS "encounterDifficulty",
                wev.success AS "encounterSuccess",
                wcv.challenge_name AS "challengeName",
                wcv.keystone_level AS "challengeKeystone",
                wcv.success AS "challengeSuccess"
            FROM squadov.matches AS m
            INNER JOIN squadov.wow_match_view AS wmv
                ON wmv.match_uuid = m.uuid
            LEFT JOIN squadov.wow_arena_view AS wav
                ON wav.view_id = wmv.id
            LEFT JOIN squadov.wow_encounter_view AS wev
                ON wev.view_id = wmv.id
            LEFT JOIN squadov.wow_challenge_view AS wcv
                ON wcv.view_id = wmv.id
            INNER JOIN squadov.users AS u
                ON u.id = wmv.user_id
            LEFT JOIN squadov.vods AS v
                ON v.match_uuid = m.uuid
                    AND v.user_uuid = u.uuid
                    AND v.is_clip = FALSE
            WHERE ${conditions.join(' AND ')}
            ORDER BY wmv.start_tm DESC
            LIMIT 20
            `,
            bind
        )

        return rows.map((ele) => {
            if (!!ele.arenaType) {
                ele.arena = {
                    type: ele.arenaType
                }
            }

            if (!!ele.encounterName) {
                ele.encounter = {
                    name: ele.encounterName,
                    difficulty: ele.encounterDifficulty,
                    success: ele.encounterSuccess
                }
            }

            if (!!ele.challengeName) {
                ele.challenge = {
                    name: ele.challengeName,
                    keystone: ele.challengeKeystone,
                    success: ele.challengeSuccess
                }
            }
            return ele
        })
    }

    async getPowerUserCurve(start, end, mode) {
        let query
        if (mode === 0) {
            query = `
                SELECT sub.days_active AS "x", COUNT(sub.user_id) AS "count"
                FROM (
                    SELECT dae.user_id AS "user_id", COUNT(DISTINCT dae.tm) AS "days_active"
                    FROM squadov.daily_active_endpoint AS dae
                    WHERE dae.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND dae.tm < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
                    GROUP BY dae.user_id
                ) sub
                GROUP BY sub.days_active
            `
        } else if (mode === 1) {
            query = `
                SELECT sub.days_active AS "x", COUNT(sub.user_id) AS "count"
                FROM (
                    SELECT v.user_uuid AS "user_id", COUNT(DISTINCT DATE_TRUNC('day', v.end_time)) AS "days_active"
                    FROM squadov.vods AS v
                    WHERE v.end_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND v.end_time < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
                        AND NOT v.is_clip
                        AND v.match_uuid IS NOT NULL
                    GROUP BY v.user_uuid
                ) sub
                GROUP BY sub.days_active
            `
        }

        const { rows } = await this.pool.query(
            query,
            [start, end]
        )

        if (rows.length === 0) {
            return []
        }
        
        let size = Math.max(...rows.map((ele) => ele.x))
        let arr = new Array(size).fill(0)
        for (let r of rows) {
            arr[r.x-1] = parseInt(r.count)
        }
        return arr
    }

    async getReferralFlow(start, end) {
        // Our endpoint are the referral codes that were used to register users
        // during this time period.
        const { rows: referralCodes } = await this.pool.query(
            `
            SELECT rc.code, rc.user_id, COUNT(u.id) AS "count"
            FROM squadov.users AS u
            INNER JOIN squadov.user_referral_code_usage AS ucu
                ON ucu.email = u.email
            INNER JOIN squadov.referral_codes AS rc
                ON rc.id = ucu.code_id
            WHERE u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time  < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
            GROUP BY rc.code, rc.user_id
            `,
            [start, end]
        )

        referralCodes.forEach((ele) => {
            ele.count = parseInt(ele.count)
        })

        // For each referral code (source), we need to know what the parent referral code (target) is so we can create the links to attribute the registration back.
        // 'No Referral' is what we should store as the referral code if no parent referral code is found. We also need to store a 'value' which indicates how many
        // attributions should be given to each link.
        let parentReferralCodes = new Map()

        let referralCodesLeftToCheck = referralCodes.filter((ele) => ele.user_id !== null).map((ele) => ele.code)
        while (referralCodesLeftToCheck.length > 0) {
            const { rows: parentCodes } = await this.pool.query(
                `
                SELECT rc.code AS "code", rc2.code AS "parent", rc2.user_id AS "puserid"
                FROM squadov.referral_codes AS rc
                INNER JOIN squadov.users AS u
                    ON u.id = rc.user_id
                INNER JOIN squadov.user_referral_code_usage AS ucu
                    ON ucu.email = u.email
                INNER JOIN squadov.referral_codes AS rc2
                    ON rc2.id = ucu.code_id
                WHERE rc.code = ANY($1::VARCHAR[])
                `,
                [referralCodesLeftToCheck]
            )

            let subMap = new Map(parentCodes.map((r) => [r.code, r.parent]))
            for (let rc of referralCodesLeftToCheck) {
                if (subMap.has(rc)) {
                    parentReferralCodes.set(rc, subMap.get(rc))
                } else {
                    parentReferralCodes.set(rc, 'No Referral')
                }
            }

            let tmp = new Set()
            for (let r of parentCodes) {
                if (r.puserid !== null ) {
                    if (!parentReferralCodes.has(r.parent)) {
                        tmp.add(r.parent)
                    }
                }
            }

            referralCodesLeftToCheck = Array.from(tmp)
        }
        
        // Key: "Source::Target", Value: # of users registered
        let uniqueSegments = new Set(['Registration'])
        let linkValues = new Map()
        let remaining = [...referralCodes]
        while (remaining.length > 0) {
            const rc = remaining.shift()

            // Should only hapen for 'No Referral' which should not have a parent target links to it.
            if (!parentReferralCodes.has(rc.code)) {
                continue
            }

            let target = parentReferralCodes.get(rc.code)
            uniqueSegments.add(rc.code)
            uniqueSegments.add(target)

            let key = `${rc.code}::${target}`
            if (!linkValues.has(key)) {
                linkValues.set(key, 0)
            }

            let val = linkValues.get(key)
            val += rc.count
            linkValues.set(key, val)

            remaining.push({
                code: target,
                count: rc.count,
            })
        }

        // Need to do another pass of the initial referral codes to tie them to the initial registration.
        for (let rc of referralCodes) {
            let key = `Registration::${rc.code}`
            linkValues.set(key, rc.count)
        }

        return {
            steps: Array.from(uniqueSegments),
            links: Array.from(linkValues).map(([key, v]) => {
                let components = key.split('::')
                return {
                    source: components[0],
                    target: components[1],
                    value: v,
                }
            })
        }
    }

    async getRegistrationBreakdown(start, end) {
        const { rows } = await this.pool.query(
            `
            SELECT COALESCE(rc.code, '[NO REFERRAL CODE]') AS "code", COUNT(u.id) AS "count"
            FROM squadov.users AS u
            LEFT JOIN squadov.user_referral_code_usage AS ucu
                ON ucu.email = u.email
            LEFT JOIN squadov.referral_codes AS rc
                ON rc.id = ucu.code_id
            WHERE u.registration_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND u.registration_time  < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
            GROUP BY rc.code
            ORDER BY COUNT(u.id) DESC
            `,
            [start, end]
        )

        let total = rows.reduceRight((a, b) => a + parseInt(b.count), 0)
        rows.forEach((ele) => {
            ele.count = parseInt(ele.count)
            ele.perc = ele.count / total
        })
        return {
            headers: ['Code', 'Count', 'Percentage'],
            data: rows.map((ele) => {
                return [
                    ele.code,
                    ele.count,
                    ele.perc,
                ]
            })
        }
    }

    async getWatchTimeGameBreakdown(start, end) {
        const { rows } = await this.pool.query(
            `
            SELECT m.game AS "game", SUM(vwa.end_seconds - vwa.start_seconds)::FLOAT / 3600.0 AS "time"
            FROM squadov.vod_watch_analytics AS vwa
            INNER JOIN squadov.vods AS v
                ON v.video_uuid = vwa.video_uuid
            INNER JOIN squadov.matches AS m
                ON m.uuid = v.match_uuid
            WHERE vwa.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND vwa.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                AND NOT v.is_clip
                    AND v.end_time IS NOT NULL
            GROUP BY m.game
            `,
            [start, end]
        )

        return {
            headers: ['Game', 'Time'],
            data: rows.map((ele) => {
                return [
                    gameToString(ele.game),
                    ele.time,
                ]
            })
        }
    }

    async getWatchTimeUserBreakdown(start, end) {
        const { rows } = await this.pool.query(
            `
            SELECT u.username AS "username", SUM(vwa.end_seconds - vwa.start_seconds)::FLOAT / 3600.0 AS "time"
            FROM squadov.vod_watch_analytics AS vwa
            INNER JOIN squadov.vods AS v
                ON v.video_uuid = vwa.video_uuid
            INNER JOIN squadov.users AS u
                ON u.uuid = v.user_uuid
            WHERE vwa.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND vwa.tm < DATE_TRUNC('day', $2::TIMESTAMPTZ)
                AND NOT v.is_clip
                    AND v.end_time IS NOT NULL
            GROUP BY u.username
            `,
            [start, end]
        )

        return {
            headers: ['Username', 'Time'],
            data: rows.map((ele) => {
                return [
                    ele.username,
                    ele.time,
                ]
            })
        }
    }

    async getActiveUserGameBreakdown(start, end) {
        const { rows } = await this.pool.query(
            `
            SELECT m.game AS "game", COUNT(DISTINCT dae.user_id) AS "count"
            FROM squadov.daily_active_endpoint AS dae
            INNER JOIN squadov.users AS u
                ON u.id = dae.user_id
            INNER JOIN squadov.vods AS v
                ON v.user_uuid = u.uuid
            INNER JOIN squadov.matches AS m
                ON m.uuid = v.match_uuid
            WHERE NOT v.is_clip
                AND v.end_time IS NOT NULL
                AND dae.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND dae.tm < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
                AND v.end_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND v.end_time < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
            GROUP BY m.game
            `,
            [start, end]
        )

        return {
            headers: ['Game', 'Count'],
            data: rows.map((ele) => {
                return [
                    gameToString(ele.game),
                    ele.count,
                ]
            })
        }
    }

    async getActivityCorrelation(start, end, mode) {
        // Y-axis
        let cte = `
            SELECT dae.user_id AS "user_id", COUNT(DISTINCT dae.tm) AS "days_active"
            FROM squadov.daily_active_endpoint AS dae
            WHERE dae.tm >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND dae.tm < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
            GROUP BY dae.user_id
        `

        let primary = `
            WITH cte AS (
                ${cte}
            )
        `

        switch (mode) {
            case 0:
                // Account age
                primary += `
                    SELECT DATE_PART('day', NOW() - u.registration_time)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                    FROM cte
                    INNER JOIN squadov.users AS u
                        ON u.id = cte.user_id
                `
                break
            case 1:
                // Days of VOD recordings
                primary += `
                    SELECT COUNT(DISTINCT DATE_TRUNC('day', v.end_time))::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                    FROM cte
                    INNER JOIN squadov.users AS u
                        ON u.id = cte.user_id
                    INNER JOIN squadov.vods AS v
                        ON v.user_uuid = u.uuid
                    WHERE v.end_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND v.end_time < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
                        AND NOT v.is_clip
                        AND v.match_uuid IS NOT NULL
                    GROUP BY v.user_uuid, cte.days_active
                `
                break
            case 2:
                // # of squads
                primary += `
                    SELECT COUNT(sra.squad_id)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                    FROM cte 
                    INNER JOIN squadov.squad_role_assignments AS sra
                        ON sra.user_id = cte.user_id
                    GROUP BY cte.user_id, cte.days_active
                `
                break
            case 3:
                // # of squads not public
                primary += `
                    SELECT COUNT(sra.squad_id)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                    FROM cte 
                    INNER JOIN squadov.squad_role_assignments AS sra
                        ON sra.user_id = cte.user_id
                    INNER JOIN squadov.squads AS s
                        ON s.id = sra.squad_id
                    WHERE NOT s.is_public AND NOT s.is_discoverable
                    GROUP BY cte.user_id, cte.days_active
                `
                break
            case 4:
                // # of squadmates
                primary += `
                SELECT COUNT(sra2.user_id)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte 
                INNER JOIN squadov.squad_role_assignments AS sra
                    ON sra.user_id = cte.user_id
                INNER JOIN squadov.squad_role_assignments AS sra2
                    ON sra2.squad_id = sra.squad_id
                GROUP BY cte.user_id, cte.days_active
                `
                break
            case 5:
                // # of squadmates (non-public)
                primary += `
                SELECT COUNT(sra2.user_id)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte 
                INNER JOIN squadov.squad_role_assignments AS sra
                    ON sra.user_id = cte.user_id
                INNER JOIN squadov.squads AS s
                    ON s.id = sra.squad_id
                INNER JOIN squadov.squad_role_assignments AS sra2
                    ON sra2.squad_id = sra.squad_id
                WHERE NOT s.is_public AND NOT s.is_discoverable
                GROUP BY cte.user_id, cte.days_active
                `
                break
            case 6:
                // # of VODs
                primary += `
                SELECT COUNT(v.video_uuid)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.users AS u
                    ON u.id = cte.user_id
                INNER JOIN squadov.vods AS v
                    ON v.user_uuid = u.uuid
                WHERE v.end_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND v.end_time < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
                    AND NOT v.is_clip
                    AND v.match_uuid IS NOT NULL
                GROUP BY v.user_uuid, cte.days_active
                `
                break
            case 7:
                // # of clips
                primary += `
                SELECT COUNT(v.video_uuid)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.users AS u
                    ON u.id = cte.user_id
                INNER JOIN squadov.vods AS v
                    ON v.user_uuid = u.uuid
                WHERE v.end_time >= DATE_TRUNC('day', $1::TIMESTAMPTZ) AND v.end_time < (DATE_TRUNC('day', $2::TIMESTAMPTZ))
                    AND v.is_clip
                    AND v.match_uuid IS NOT NULL
                GROUP BY v.user_uuid, cte.days_active
                `
                break
            case 8:
                // # of share links (no clips)
                primary += `
                SELECT COUNT(st.id)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.share_tokens AS st
                    ON st.user_id = cte.user_id
                WHERE st.clip_uuid IS NULL
                GROUP BY cte.user_id, cte.days_active
                `
                break
            case 9:
                // # of share links (with clips)
                primary += `
                SELECT COUNT(st.id)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.share_tokens AS st
                    ON st.user_id = cte.user_id
                GROUP BY cte.user_id, cte.days_active
                `
                break
            case 10:
                // Most played game
                primary += `
                SELECT DISTINCT ON (sub.user_id)
                    CASE
                        WHEN sub.game = 0 THEN 'Aim Lab'
                        WHEN sub.game = 1 THEN 'Hearthstone'
                        WHEN sub.game = 2 THEN 'League of Legends'
                        WHEN sub.game = 3 THEN 'Teamfight Tactics'
                        WHEN sub.game = 4 THEN 'Valorant'
                        WHEN sub.game = 5 THEN 'World of Warcraft'
                        WHEN sub.game = 6 THEN 'CSGO'
                        ELSE 'Unknown'
                    END AS "x",
                    sub.y::INTEGER AS "y"
                FROM (
                    SELECT cte.user_id, m.game, SUM(EXTRACT(EPOCH FROM (v.end_time - v.start_time))) AS "count", cte.days_active::INTEGER AS "y"
                    FROM cte
                    INNER JOIN squadov.users AS u
                        ON u.id = cte.user_id
                    INNER JOIN squadov.vods AS v
                        ON v.user_uuid = u.uuid
                    INNER JOIN squadov.matches AS m
                        ON m.uuid = v.match_uuid
                    WHERE NOT v.is_clip
                        AND v.end_time IS NOT NULL
                    GROUP BY cte.user_id, cte.days_active, m.game
                ) sub
                ORDER BY sub.user_id, sub.count DESC
                `
                break
            case 11:
                // Internet speed
                primary += `
                SELECT ROUND(u.speed_check_mbps)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.users AS u
                    ON u.id = cte.user_id
                `
                break
            case 12:
                // CPU
                primary += `
                SELECT uhs.cpu->>'brand' AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.user_hardware_specs AS uhs
                    ON uhs.user_id = cte.user_id
                `
                break
            case 13:
                // GPU
                primary += `
                SELECT uhs.display#>>'{gpus, 0, name}' AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.user_hardware_specs AS uhs
                    ON uhs.user_id = cte.user_id
                `
                break
            case 14:
                // Verified
                primary += `
                SELECT
                    CASE u.verified 
                        WHEN TRUE THEN 'Yes'
                        ELSE 'No'
                    END AS "x",
                    cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.users AS u
                    ON u.id = cte.user_id
                `
                break
            case 15:
                // # of squads public
                primary += `
                SELECT COUNT(sra.squad_id)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte 
                INNER JOIN squadov.squad_role_assignments AS sra
                    ON sra.user_id = cte.user_id
                INNER JOIN squadov.squads AS s
                    ON s.id = sra.squad_id
                WHERE s.is_public AND s.is_discoverable
                GROUP BY cte.user_id, cte.days_active
                `
                break
            case 16:
                // # of hours of VOD recordings
                primary += `
                SELECT ROUND(SUM(EXTRACT(EPOCH FROM (v.end_time - v.start_time)))::DOUBLE PRECISION / 60.0 / 60.0)::INTEGER AS "x", cte.days_active::INTEGER AS "y"
                FROM cte
                INNER JOIN squadov.users AS u
                    ON u.id = cte.user_id
                INNER JOIN squadov.vods AS v
                    ON v.user_uuid = u.uuid
                INNER JOIN squadov.matches AS m
                    ON m.uuid = v.match_uuid
                WHERE NOT v.is_clip
                    AND v.end_time IS NOT NULL
                GROUP BY cte.user_id, cte.days_active
                `
                break
        }

        const { rows } = await this.pool.query(
            primary,
            [start, end]
        )

        return rows.map((ele) => {
            return {
                x: ele.x,
                y: ele.y,
            }
        })
    }
}

exports.ApiServer = ApiServer