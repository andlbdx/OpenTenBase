/*
 * src/test/examples/testlibpq.c
 *
 *
 * testlibpq.c
 *
 *        Test the C version of libpq, the PostgreSQL frontend library.
 */
#include <stdio.h>
#include <stdlib.h>
#include "libpq-fe.h"

static void
exit_nicely(PGconn *conn)
{
    PQfinish(conn); // 关闭数据库连接
    exit(1); // 退出程序
}

int
main(int argc, char **argv)
{
    const char *conninfo;
    PGconn       *conn;
    PGresult   *res;
    int            nFields;
    int            i,
                j;

    /*
     * If the user supplies a parameter on the command line, use it as the
     * conninfo string; otherwise default to setting dbname=postgres and using
     * environment variables or defaults for all other connection parameters.
     */
     // 检查命令行参数以确定连接信息
    if (argc > 1)
        conninfo = argv[1];
    else
        conninfo = "dbname = postgres";  // 默认连接信息

    /* Make a connection to the database */
    // 建立到数据库的连接
    conn = PQconnectdb(conninfo);

    /* Check to see that the backend connection was successfully made */
    /* 检查数据库连接是否成功 */
    if (PQstatus(conn) != CONNECTION_OK)
    {
        fprintf(stderr, "Connection to database failed: %s",
                PQerrorMessage(conn));// 输出连接失败的错误信息
        exit_nicely(conn); // 退出程序并关闭数据库连接
    }

    /*
     * Our test case here involves using a cursor, for which we must be inside
     * a transaction block.  We could do the whole thing with a single
     * PQexec() of "select * from pg_database", but that's too trivial to make
     * a good example.
     */

    /* Start a transaction block */
    // 在事务块中执行操作
    res = PQexec(conn, "BEGIN");  // 开始事务
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    { 
        fprintf(stderr, "BEGIN command failed: %s", PQerrorMessage(conn));  // 输出失败信息
        PQclear(res);// 释放结果对象
        exit_nicely(conn);// 退出程序并关闭数据库连接
    }

    /*
     * Should PQclear PGresult whenever it is no longer needed to avoid memory
     * leaks
     */
    PQclear(res);

    /*
     * Fetch rows from pg_database, the system catalog of databases
     */
     // 声明一个游标
    res = PQexec(conn, "DECLARE myportal CURSOR FOR select * from pg_database");
    if (PQresultStatus(res) != PGRES_COMMAND_OK)
    {
        // 输出失败信息
        fprintf(stderr, "DECLARE CURSOR failed: %s", PQerrorMessage(conn));
        // 释放结果对象
        PQclear(res);
        // 退出程序并关闭数据库连接
        exit_nicely(conn);
    }
    // 释放结果对象
    PQclear(res);

    // 获取游标中的数据
    res = PQexec(conn, "FETCH ALL in myportal");
    if (PQresultStatus(res) != PGRES_TUPLES_OK)
    {
        // 输出失败信息
        fprintf(stderr, "FETCH ALL failed: %s", PQerrorMessage(conn));
        // 释放结果对象
        PQclear(res);
        // 退出程序并关闭数据库连接
        exit_nicely(conn);
    }

    /* first, print out the attribute names */
    // 打印列名
    nFields = PQnfields(res);
    for (i = 0; i < nFields; i++)
        printf("%-15s", PQfname(res, i));
    printf("\n\n");

    /* next, print out the rows */
    // 打印数据行
    for (i = 0; i < PQntuples(res); i++)
    {
        for (j = 0; j < nFields; j++)
            printf("%-15s", PQgetvalue(res, i, j));
        printf("\n");
    }

    // 释放结果对象
    PQclear(res);

    /* close the portal ... we don't bother to check for errors ... */
    // 关闭游标
    res = PQexec(conn, "CLOSE myportal");
    // 释放结果对象
    PQclear(res);

    /* end the transaction */
    // 结束事务
    res = PQexec(conn, "END");
    // 释放结果对象
    PQclear(res);

    /* close the connection to the database and cleanup */
    // 关闭数据库连接
    PQfinish(conn);

    return 0;
}