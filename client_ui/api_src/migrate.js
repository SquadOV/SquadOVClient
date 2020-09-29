const CURRENT_DB_VERSION = 5

async function migrateDb(db) {
    return new Promise(resolve => {
        db.get('PRAGMA user_version', [], (err, row) => {
            if (!!err) {
                console.log('poop prgrama')
                throw err
            }
    
            const currentVersion = row.user_version
            console.log(`Migrating DB from v${currentVersion} to v${CURRENT_DB_VERSION}`)
            db.serialize(() => {
                db.run('BEGIN EXCLUSIVE TRANSACTION;')
                if (currentVersion < 1) {
                    // Valorant account data.
                    db.run(`
CREATE TABLE valorant_accounts (
    puuid TEXT PRIMARY KEY,
    username TEXT NOT NULL,
    tag TEXT NOT NULL
);
                    `)
                }

                if (currentVersion < 2) {
                    // Valorant match history
                    db.run(`
CREATE TABLE valorant_matches (
    id TEXT PRIMARY KEY,
    gameMode TEXT NOT NULL,
    map TEXT NOT NULL,
    is_ranked INTEGER NOT NULL,
    provisioning_flow_id TEXT NOT NULL,
    game_version TEXT NOT NULL,
    server_start_time_utc INTEGER NOT NULL,
    start_time_utc TEXT,
    end_time_utc TEXT
);
                    `)

                    db.run(`
CREATE TABLE valorant_match_teams (
    match_id TEXT NOT NULL REFERENCES valorant_matches(id) ON DELETE CASCADE,
    team_id TEXT NOT NULL,
    won INTEGER NOT NULL,
    rounds_won INTEGER NOT NULL,
    rounds_played INTEGER NOT NULL,
    PRIMARY KEY(match_id, team_id)
);                   
                    `)

                    db.run(`
CREATE TABLE valorant_match_players (
    match_id TEXT NOT NULL REFERENCES valorant_matches(id) ON DELETE CASCADE,
    team_id TEXT NOT NULL,
    puuid TEXT NOT NULL,
    agent_id TEXT NOT NULL,
    competitive_tier INTEGER NOT NULL,
    total_combat_score INTEGER NOT NULL,
    rounds_played INTEGER NOT NULL,
    kills INTEGER NOT NULL,
    deaths INTEGER NOT NULL,
    assists INTEGER NOT NULL,
    PRIMARY KEY (match_id, puuid),
    FOREIGN KEY (match_id, team_id) REFERENCES valorant_match_teams(match_id, team_id) ON DELETE CASCADE
);
`)

                    db.run(`
CREATE TABLE valorant_match_rounds (
    match_id TEXT NOT NULL REFERENCES valorant_matches(id) ON DELETE CASCADE,
    round_num INTEGER NOT NULL,
    plant_round_time INTEGER,
    planter_puuid TEXT,
    defuse_round_time INTEGER,
    defuser_puuid TEXT,
    team_round_winner TEXT NOT NULL,
    round_buy_time_utc TEXT,
    round_play_time_utc TEXT,
    PRIMARY KEY (match_id, round_num),
    FOREIGN KEY (match_id, planter_puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE,
    FOREIGN KEY (match_id, defuser_puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE,
    FOREIGN KEY (match_id, team_round_winner) REFERENCES valorant_match_teams(match_id, team_id) ON DELETE CASCADE
);
`)                   
                    db.run(`
CREATE TABLE valorant_match_round_player_loadout (
    match_id TEXT NOT NULL REFERENCES valorant_matches(id) ON DELETE CASCADE,
    round_num INTEGER NOT NULL,
    puuid TEXT NOT NULL,
    loadout_value INTEGER NOT NULL,
    remaining_money INTEGER NOT NULL,
    spent_money INTEGER NOT NULL,
    weapon TEXT NOT NULL,
    armor TEXT NOT NULL,
    FOREIGN KEY (match_id, puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE,
    FOREIGN KEY (match_id, round_num) REFERENCES valorant_match_rounds(match_id, round_num) ON DELETE CASCADE
);
`) 
                    db.run(`
CREATE TABLE valorant_match_kill (
    match_id TEXT NOT NULL REFERENCES valorant_matches(id) ON DELETE CASCADE,
    round_num INTEGER NOT NULL,
    killer_puuid TEXT NOT NULL,
    victim_puuid TEXT NOT NULL,
    round_time INTEGER NOT NULL,
    damage_type TEXT NOT NULL,
    damage_item TEXT NOT NULL,
    is_secondary_fire INTEGER NOT NULL,
    FOREIGN KEY (match_id, round_num) REFERENCES valorant_match_rounds(match_id, round_num) ON DELETE CASCADE,
    FOREIGN KEY (match_id, killer_puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE,
    FOREIGN KEY (match_id, victim_puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE
);
`)

                    db.run(`
CREATE TABLE valorant_match_damage (
    match_id TEXT NOT NULL REFERENCES valorant_matches(id) ON DELETE CASCADE,
    round_num INTEGER NOT NULL,
    instigator_puuid TEXT NOT NULL,
    receiver_puuid TEXT NOT NULL,
    damage INTEGER NOT NULL,
    legshots INTEGER NOT NULL,
    bodyshots INTEGER NOT NULL,
    headshots INTEGER NOT NULL,
    FOREIGN KEY (match_id, round_num) REFERENCES valorant_match_rounds(match_id, round_num) ON DELETE CASCADE,
    FOREIGN KEY (match_id, instigator_puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE,
    FOREIGN KEY (match_id, receiver_puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE
);
`)

                    db.run(`
CREATE TABLE valorant_match_round_player_stats (
    match_id TEXT NOT NULL REFERENCES valorant_matches(id) ON DELETE CASCADE,
    round_num INTEGER NOT NULL,
    puuid TEXT NOT NULL,
    combat_score INTEGER NOT NULL,
    FOREIGN KEY (match_id, round_num) REFERENCES valorant_match_rounds(match_id, round_num) ON DELETE CASCADE,
    FOREIGN KEY (match_id, puuid) REFERENCES valorant_match_players(match_id, puuid) ON DELETE CASCADE
);
`) 
                }

                if (currentVersion < 3) {
                    db.run(`
CREATE TABLE valorant_match_videos (
    match_id TEXT NOT NULL UNIQUE REFERENCES valorant_matches(id) ON DELETE CASCADE,
    video_path TEXT NOT NULL
);
`) 
                }

                if (currentVersion < 4) {
                    // Views for easier stat grabbing
                    db.run(`
CREATE VIEW view_valorant_player_match_stats (
    matchId,
    puuid,
    rank,
    kills,
    deaths,
    assists,
    rounds,
    totalCombatScore,
    totalDamage,
    headshots,
    bodyshots,
    legshots,
    won
)
AS
SELECT
    vmp.match_id,
    vmp.puuid,
    vmp.competitive_tier,
    vmp.kills,
    vmp.deaths,
    vmp.assists,
    vmp.rounds_played,
    vmp.total_combat_score,
    SUM(vmd.damage),
    SUM(vmd.headshots),
    SUM(vmd.bodyshots),
    SUM(vmd.legshots),
    vmt.won
FROM valorant_match_players AS vmp
INNER JOIN valorant_match_teams AS vmt
    ON vmt.match_id = vmp.match_id AND vmt.team_id = vmp.team_id
LEFT JOIN valorant_match_damage AS vmd
    ON vmd.instigator_puuid = vmp.puuid AND vmd.match_id = vmp.match_id
GROUP BY vmp.match_id, vmp.puuid
`)
                }

                if (currentVersion < 5) {
                    // Aim Lab Data
                    db.run(`
CREATE TABLE aimlab_tasks (
    id INTEGER PRIMARY KEY,
    taskName TEXT NOT NULL,
    mode INTEGER NOT NULL,
    score INTEGER NOT NULL,
    createDate TEXT NOT NULL,
    version TEXT NOT NULL,
    rawData TEXT NOT NULL,
    vodPath TEXT
)                 
                    `)
                }
    
                db.run(`PRAGMA user_version = ${CURRENT_DB_VERSION}`)
                db.exec('COMMIT TRANSACTION;')
            })
            resolve()
        })
        
    })

}

exports.migrateDb = migrateDb