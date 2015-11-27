QL* connectToDatabase() {
    MYSQL *con = mysql_init(NULL);

    if (con == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        exit(1);
    }
    if (mysql_real_connect(con, "localhost", "root", "root",
        NULL, 0, NULL, 0) == NULL) {
        fprintf(stderr, "%s\n", mysql_error(con));
        mysql_close(con);
        exit(1);
    }

    if(mysql_select_db(con, "embedded")==0)/*success*/
            printf( "Database Selected\n");
    else
       printf( "Failed to connect to Database: Error: \n");

    return con;
}
