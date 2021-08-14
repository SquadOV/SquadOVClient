const { Pool, Client } = require('pg')
const fs = require('fs')
const bent = require('bent')

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
            g.cnt AS "count"
        FROM series AS s
        CROSS JOIN LATERAL (
            SELECT rc.code, COUNT(DISTINCT u.id)
            FROM squadov.users AS u
            LEFT JOIN squadov.user_referral_code_usage AS rcu
                ON rcu.email = u.email
            LEFT JOIN squadov.referral_codes AS rc
                ON rc.id = rcu.code_id
            WHERE u.registration_time IS NOT NULL AND u.registration_time >= s.tm AND u.registration_time < s.tm + INTERVAL '1 day'
            GROUP BY rc.code
        ) AS g(code, cnt)
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
                    },
                    sub: ['Users', 'Organic', ...referralCodes]
                }

                for (let r of referralCodes) {
                    d.data[r] = 0
                }
                retData.set(tmKey, d)
            }
            
            let d = retData.get(tmKey)
            let cnt = parseInt(r.count)
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
            ), 0) AS "downloads"
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
                },
                sub: ['Visits', 'Downloads']
            }
        })
    }

    async getReferralUsersBreakdown() {
        const query = `
        SELECT
            rc.code AS "code",
            u.username AS "description",
            v.value AS "visits",
            d.value AS "downloads"
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
                downloads: parseInt(r.downloads)
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
            ), 0) AS "downloads"
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
                },
                sub: ['Visits', 'Downloads']
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
                downloads: parseInt(r.downloads)
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
        }
    }

    async getCohortSize(interval, start, end) {
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
            COUNT(u.id) AS "count"
        FROM series AS s
        LEFT JOIN squadov.users AS u
            ON u.registration_time >= s.tm AND u.registration_time < s.tm + INTERVAL '1 ${pgInterval}'
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

        let cohortSizes = await this.getCohortSize(interval, start, end)
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

        let cohortSizes = await this.getCohortSize(interval, start, end)
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

        let cohortSizes = await this.getCohortSize(interval, start, end)
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

    async getFunnelData(start, end, codes) {
        const { rows } = await this.pool.query(
            `
            SELECT (
                SELECT COUNT(rv.code)
                FROM squadov.referral_visits AS rv
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rv.code
                WHERE rc.code = ANY($3)
                    AND rv.tm >= $1::TIMESTAMPTZ AND rv.tm < $2::TIMESTAMPTZ
            ) AS "view", (
                SELECT COUNT(DISTINCT rcu.email)
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= $1::TIMESTAMPTZ AND rcu.tm < $2::TIMESTAMPTZ
            ) AS "reg", (
                SELECT COUNT(DISTINCT u.id)
                FROM squadov.user_referral_code_usage AS rcu
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rcu.code_id
                INNER JOIN squadov.users AS u
                    ON u.email = rcu.email
                WHERE rc.code = ANY($3)
                    AND rcu.tm >= $1::TIMESTAMPTZ AND rcu.tm < $2::TIMESTAMPTZ
            ) AS "login", (
                SELECT COUNT(rv.code)
                FROM squadov.referral_downloads AS rv
                INNER JOIN squadov.referral_codes AS rc
                    ON rc.id = rv.code
                WHERE rc.code = ANY($3)
                    AND rv.tm >= $1::TIMESTAMPTZ AND rv.tm < $2::TIMESTAMPTZ
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
                    AND rcu.tm >= $1::TIMESTAMPTZ AND rcu.tm < $2::TIMESTAMPTZ
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
                    AND rcu.tm >= $1::TIMESTAMPTZ AND rcu.tm < $2::TIMESTAMPTZ
                    AND v.is_clip = FALSE
                    AND v.end_time IS NOT NULL
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
                    AND rcu.tm >= $1::TIMESTAMPTZ AND rcu.tm < $2::TIMESTAMPTZ
            ) AS "active"
            `,
            [start, end, codes]
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
                        SELECT DISTINCT lta.twitch_name AS "name"
                        FROM squadov.linked_twitch_accounts AS lta
                        WHERE lta.user_id = $1
                        `,
                        [rows[0].id]
                    )).rows.map((ele) => ele.name),
                    wowChars: (await this.pool.query(
                        `
                        SELECT DISTINCT wcp.unit_name AS "name"
                        FROM squadov.wow_user_character_cache AS wucc
                        INNER JOIN squadov.wow_match_view_combatants AS wvc
                            ON wvc.event_id = wucc.event_id
                        INNER JOIN squadov.wow_match_view_character_presence AS wcp
                            ON wcp.character_id = wvc.character_id
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
            SELECT
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
            INNER JOIN squadov.vod_metadata AS vm
                ON vm.video_uuid = v.video_uuid
            LEFT JOIN squadov.share_tokens AS st
                ON st.match_uuid = v.match_uuid OR st.clip_uuid = v.video_uuid
            WHERE ${condition}
            ORDER BY v.start_time DESC
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
}

exports.ApiServer = ApiServer