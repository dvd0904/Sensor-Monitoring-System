#include "fdb.h"

static const char *SQL_STMT[] = {
    [FDB_STMT_DATA_INSERT]          = "INSERT INTO sensor (sensor_id, temperature, ts) VALUES (?, ?, ?);",
    [FDB_STMT_DATA_GET_SID]         = "SELECT * FROM sensor WHERE sensor_id = ?;",
    [FDB_STMT_DATA_GET_TEMP_ASC]    = "SELECT * FROM sensor WHERE sensor_id = ? ORDER BY temperature;",
};

/**
 * @brief Create a database file
 * @param path path to database file name
 * @param source script from sql file
 * @return 0 if success otherwise 0
 */
static int fdb_create_file(const char *path, const char *source);

fdb_t *fdb_init(sqlite3 *db)
{
    fdb_t *fdb;
    fdb = (fdb_t *)calloc(1, sizeof(fdb_t));
    fdb->db = db;
    return fdb;
}

void fdb_destroy(fdb_t *fdb)
{
    if(sqlite3_close_v2(fdb->db) == SQLITE_OK)
        mdebug(DB_DISCONNECT);
        
    free(fdb);
}

static int fdb_create_file(const char *path, const char *source)
{
    const char *sql;
    const char *tail;
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int result;

    if (sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL))
    {
        mdebug("Couldn't create SQLite database '%s': %s.", path, sqlite3_errmsg(db));
        sqlite3_close_v2(db);
        return -1;
    }

    for (sql = source; sql && *sql; sql = tail)
    {
        if (sqlite3_prepare_v2(db, sql, -1, &stmt, &tail) != SQLITE_OK)
        {
            mdebug("Preparing statement: %s.", sqlite3_errmsg(db));
            sqlite3_close_v2(db);
            return -1;
        }

        result = sqlite3_step(stmt);

        switch (result)
        {
        case SQLITE_MISUSE:
        case SQLITE_ROW:
        case SQLITE_DONE:
            break;
        default:
            mdebug("Stepping statement: %s.", sqlite3_errmsg(db));
            sqlite3_finalize(stmt);
            sqlite3_close_v2(db);
            return -1;
        }

        sqlite3_finalize(stmt);
    }

    sqlite3_close_v2(db);

    if (chmod(path, 0640) < 0)
    {
        merror(CHMOD_ERROR, path, errno, strerror(errno));
        return -1;
    }


    return 0;
}

int fdb_create_sensor_db()
{
    char path[256 + 1];
    char buffer[4096];
    FILE *source;
    FILE *dest;
    size_t nbytes;
    int result = 0;

    snprintf(path, 256, "%s", ".template.db");

    if (!(source = fopen(path, "r")))
    {
        mdebug("Profile database not found, creating.");

        if (fdb_create_file(path, schema_sensor_sql) < 0)
            return -1;

        // Retry to open
        if (!(source = fopen(path, "r")))
        {
            merror("Couldn't open profile '%s'.", path);
            return -1;
        }
    }

    snprintf(path, 256, "%s.db", "sensor");

    if (!(dest = fopen(path, "w")))
    {
        merror("Couldn't create database '%s': %s (%d).", path, strerror(errno), errno);
        fclose(source);
        return -1;
    }


    while (nbytes = fread(buffer, 1, 4096, source), nbytes)
    {
        if (fwrite(buffer, 1, nbytes, dest) != nbytes)
        {
            unlink(path);
            result = -1;
            break;
        }
    }

    fclose(source);
    if (fclose(dest) == -1)
    {
        merror("Couldn't create file %s completely.", path);
        return -1;
    }

    if (result < 0)
    {
        unlink(path);
        return -1;
    }

    if (chmod(path, 0640) < 0)
    {
        merror(CHMOD_ERROR, path, errno, strerror(errno));
        unlink(path);
        return -1;
    }

    mdebug(DB_CREATE, path);

    return 0;

}

fdb_t *fdb_open_sensor_db()
{
    sqlite3 *db;
    fdb_t *fdb = NULL;
    const char *path = "sensor.db";
    
    if (sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE, NULL))
    {
        mdebug("No SQLite database found, creating.");
        sqlite3_close_v2(db);

        if (fdb_create_sensor_db() < 0)
        {
            merror("Couldn't create SQLite database '%s'", path);
            goto end;
        }

        // Retry to open
        if (sqlite3_open_v2(path, &db, SQLITE_OPEN_READWRITE, NULL))
        {
            merror("Can't open SQLite database '%s': %s", path, sqlite3_errmsg(db));
            sqlite3_close_v2(db);
            goto end;
        }

        fdb = fdb_init(db);
    }
    else 
    {
        fdb = fdb_init(db);
        if(fdb == NULL)
            goto end;

    }

end:
    return fdb;
}

int fdb_stmt_cache(fdb_t *fdb, int index)
{
    if (index >= FDB_STMT_SIZE)
    {
        merror("DB SQL statement index (%d) out of bounds.", index);
        return -1;
    }


    if (!fdb->stmt[index])
    {
        if (sqlite3_prepare_v2(fdb->db, SQL_STMT[index], -1, fdb->stmt + index, NULL) != SQLITE_OK)
        {
            merror("DB sqlite3_prepare_v2() stmt(%d): %s", index, sqlite3_errmsg(fdb->db));
            return -1;
        }
    }
    else
    {
        sqlite3_reset(fdb->stmt[index]);
        sqlite3_clear_bindings(fdb->stmt[index]);
    }

    return 0;
}

int fdb_data_insert(fdb_t *fdb, int SID, double temp, const char *ts)
{
    sqlite3_stmt *stmt = NULL;

    if (fdb_stmt_cache(fdb, FDB_STMT_DATA_INSERT) < 0)
    {
        merror("at fdb_data_insert(): cannot cache statement");
        return -1;
    }

    stmt = fdb->stmt[FDB_STMT_DATA_INSERT];

    sqlite3_bind_int(stmt, 1, SID);
    sqlite3_bind_double(stmt, 2, temp);
    sqlite3_bind_text(stmt, 3, ts, -1, NULL);

    if (sqlite3_step(stmt) == SQLITE_DONE)
        return 0;
    else
    {
        merror("at fdb_data_insert(): sqlite3_step(): %s", sqlite3_errmsg(fdb->db));
        return -1;
    }

}