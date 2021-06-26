const { Pool, Client } = require('pg')

class ApiServer {
    constructor(config) {
        this.pool = new Pool({
            user: config.database.username,
            password: config.database.password,
            host: config.database.host,
            database: config.database.db,
            port: config.database.port,
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
                    DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                    DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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
                    DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                    DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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

    async getNewlyRegisteredUsers(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(DISTINCT id)
                FROM squadov.users AS u
                WHERE u.registration_time IS NOT NULL AND u.registration_time >= s.tm AND u.registration_time < s.tm + INTERVAL '1 ${pgInterval}'
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
                    DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                    DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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
        let registeredUsers = await this.getNewlyRegisteredUsers(interval, start, end)
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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

    async getAimLabGames(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.aimlab_tasks AS m
                WHERE m.create_date >= s.tm AND m.create_date < s.tm + INTERVAL '1 ${pgInterval}'
            ), 0) AS "games"
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
                    'Games': parseInt(r.games),
                },
                sub: ['Games']
            }
        })
    }

    async getHearthstoneGames(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.hearthstone_matches AS m
                WHERE m.match_time >= s.tm AND m.match_time < s.tm + INTERVAL '1 ${pgInterval}'
            ), 0) AS "games"
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
                    'Games': parseInt(r.games),
                },
                sub: ['Games']
            }
        })
    }

    async getLolGames(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.lol_matches AS m
                WHERE m.game_start_time >= s.tm AND m.game_start_time < s.tm + INTERVAL '1 ${pgInterval}'
            ), 0) AS "games"
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
                    'Games': parseInt(r.games),
                },
                sub: ['Games']
            }
        })
    }

    async getTftGames(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.tft_matches AS m
                WHERE m.game_start_time >= s.tm AND m.game_start_time < s.tm + INTERVAL '1 ${pgInterval}'
            ), 0) AS "games"
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
                    'Games': parseInt(r.games),
                },
                sub: ['Games']
            }
        })
    }

    async getValorantGames(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.valorant_matches AS m
                WHERE m.server_start_time_utc >= s.tm AND m.server_start_time_utc < s.tm + INTERVAL '1 ${pgInterval}'
            ), 0) AS "games"
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
                    'Games': parseInt(r.games),
                },
                sub: ['Games']
            }
        })
    }

    async getWowGames(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.wow_match_view AS m
                WHERE m.end_tm >= s.tm AND m.end_tm < s.tm + INTERVAL '1 ${pgInterval}'
            ), 0) AS "games"
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
                    'Games': parseInt(r.games),
                },
                sub: ['Games']
            }
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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

    async getVods(interval, start, end) {
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
                INTERVAL '1 ${pgInterval}'
            )
        )
        SELECT
            s.tm AS "tm",
            COALESCE((
                SELECT COUNT(1)
                FROM squadov.vods AS m
                WHERE m.end_time >= s.tm AND m.end_time < s.tm + INTERVAL '1 ${pgInterval}'
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
                    'VODs': parseInt(r.squads),
                },
                sub: ['VODs']
            }
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

    async getMetrics(metric, interval, start, end) {
        switch (metric) {
            case 0:
                return await this.getActiveUsers(interval, start, end)
            case 1:
                return await this.getChurn(interval, start, end)
            case 2:
                return await this.getReferralUsersMetric(interval, start, end)
            case 3:
                return await this.getReferralCampaignsMetric(interval, start, end)
            case 4: // Aim Lab
                return await this.getAimLabGames(interval, start, end)
            case 5: // Hearthstone
                return await this.getHearthstoneGames(interval, start, end)
            case 6: // LoL
                return await this.getLolGames(interval, start, end)
            case 7: // TFT
                return await this.getTftGames(interval, start, end)
            case 8: // Valorant
                return await this.getValorantGames(interval, start, end)
            case 9: // WoW
                return await this.getWowGames(interval, start, end)
            case 10: // Registrations
                return await this.getNewlyRegisteredUsers(interval, start, end)
            case 11: // Squad Invites
                return await this.getSquadInvites(interval, start, end)
            case 12: // Squads
                return await this.getSquads(interval, start, end)
            case 13: // VODs
                return await this.getVods(interval, start, end)
            case 14: // Lost Users
                return await this.getLostUsers(interval, start, end)
            case 15: // Average Age
                return await this.getAverageAgeKpi(interval, start, end)
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
                DATE_TRUNC('${pgInterval}', $1::TIMESTAMPTZ),
                DATE_TRUNC('${pgInterval}', $2::TIMESTAMPTZ),
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
}

exports.ApiServer = ApiServer