/*
 * Copyright © 2001-2010 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <modbus.h>
#include <mysql.h>
#include <my_global.h>

/* The goal of this program is to check all major functions of
   libmodbus:
   - write_coil
   - read_bits
   - write_coils
   - write_register
   - read_registers
   - write_registers
   - read_registers

   All these functions are called with random values on a address
   range defined by the following defines.
*/
#define LOOP             1
#define SERVER_ID        1
#define ADDRESS_START    0
#define ADDRESS_END      2

/* At each loop, the program works in the range ADDRESS_START to
 * ADDRESS_END then ADDRESS_START + 1 to ADDRESS_END and so on.
 */
int main(void)
{
    modbus_t *ctx;
    int rc;
    int nb_fail;
    int nb_loop;
    int addr;
    int nb;
    uint16_t *tab_rp_registers;
    char query[100];
    int reg_address;


    /* RTU */
    ctx = modbus_new_rtu("/dev/ttyUSB0", 9600, 'N', 8, 1);
    modbus_set_slave(ctx, SERVER_ID);

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }


     MYSQL *con = mysql_init(NULL);
    
	if (con == NULL) 
	  {
	      fprintf(stderr, "%s\n", mysql_error(con));
	      exit(1);
	  }

	  if (mysql_real_connect(con, "localhost", "root", "root", 
	          NULL, 0, NULL, 0) == NULL) 
	  {
	      fprintf(stderr, "%s\n", mysql_error(con));
	      mysql_close(con);
	      exit(1);
	  }  

	if(mysql_select_db(con, "embedded")==0)/*success*/
	    printf( "Database Selected\n");
	else
	    printf( "Failed to connect to Database: Error: \n");

	if (mysql_query(con, "select address from configuration")) {
	      fprintf(stderr, "%s\n", mysql_error(con));
	      exit(1);
	   }

	MYSQL_RES * res = mysql_use_result(con);
	
	 nb =  mysql_num_rows(res);
         tab_rp_registers = (uint16_t *) malloc(nb * sizeof(uint16_t));
	 memset(tab_rp_registers, 0, nb * sizeof(uint16_t));

	 MYSQL_ROW row;
	   /* output table name */
	   printf("Addresses from database:\n");
   	   while ((row = mysql_fetch_row(res)) != NULL) {

		reg_address = atoi(row[0]);
                addr = reg_address - 30000;
      		printf("%i \n", addr);
 		rc = modbus_read_input_registers(ctx, addr, 1, tab_rp_registers);

                if (rc == -1) {
                    printf("ERROR modbus_read_input_registers (%d)\n", rc);
                    nb_fail++;
                } else{
                    printf("Address = %d, value %d \n",addr, tab_rp_registers[0]);
                }

		sprintf(query, "INSERT INTO solar_panel_data (address, value) VALUES (%i, %d)", reg_address, tab_rp_registers[0]);
		printf("%s\n", query);
/*		if (mysql_query(con, "INSERT INTO solar_panel_data (address, value) VALUES (30001, 65273)")) {
			printf("ERROR writing to database");
  		}
*/	   }
	   mysql_free_result(res);
//do the queries in to matrix
 if (mysql_query(con, "INSERT INTO solar_panel_data (address, value) VALUES (30001, 65273)")) {
                        printf("ERROR writing to database");
                }

 
    /* Free the memory */
    free(tab_rp_registers);

    /* Close the connection */
    modbus_close(ctx);
    modbus_free(ctx);
    mysql_close(con);

    return 0;
}
