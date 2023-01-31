/*
 * Created by HelliWrold1 on 2023/1/30 21:24.
 */
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "mariadb_connector.h"

TEST_CASE("connectMariadbSuccess")
{
    MariadbConnector_t mariadbConn;
    initMariadbConnector(&mariadbConn);
    mariadbConn.table = "AD123";
    CHECK(MARIADB_CONNECTOR_SUCCESS == connectMariadb(&mariadbConn));
    SUBCASE("mariadbCreateTableSuccess")
    {
        CHECK(MARIADB_CONNECTOR_SUCCESS == mariadbCreateTable(&mariadbConn));
    }
//    SUBCASE("mariadbCreateTableFailure")
//    {
//        CHECK(MARIADB_CONNECTOR_CONNECT_FAILURE == mariadbCreateTable(&mariadbConn));
//    }
    SUBCASE("mariadbInsertRecordSuccess")
    {
        CHECK(MARIADB_CONNECTOR_SUCCESS == mariadbInsertRecord(&mariadbConn,
                                                                    "{"
                                                                     "    \"app\": \"Raspiber-handler\","
                                                                     "    \"battery\": 0,"
                                                                     "    \"codr\": \"4/5\","
                                                                     "    \"data\": \"0012345678901234\","
                                                                     "    \"data1\": 18,"
                                                                     "    \"data2\": 52,"
                                                                     "    \"data3\": 86,"
                                                                     "    \"data4\": 120,"
                                                                     "    \"data5\": 144,"
                                                                     "    \"data6\": 18,"
                                                                     "    \"data7\": 52,"
                                                                     "    \"datatype\": 0,"
                                                                     "    \"datetime\": \"2023-01-27T18:14:02Z\","
                                                                     "    \"datr\": \"SF12BW125\","
                                                                     "    \"desc\": \"ABP-Ra-08-Control\","
                                                                     "    \"devaddr\": \"67678D5E\","
                                                                     "    \"fcnt\": 1,"
                                                                     "    \"freq\": 474.9,"
                                                                     "    \"lsnr\": -11.2,"
                                                                     "    \"mac\": \"B827EBFFFE2114B5\","
                                                                     "    \"port\": 2,"
                                                                     "    \"rssi\": -115"
                                                                     "}", 0));
    }
    SUBCASE("mariadbQueryRecordSuccess")
    {
        MariadbQueryResult_t mariadbRes;
        initMariadbQueryResult(&mariadbRes);
        CHECK(MARIADB_CONNECTOR_SUCCESS == mariadbQueryRecord(&mariadbConn,&mariadbRes));
        SUBCASE("mariadbReplaceTableSuccess")
        {
            CHECK(MARIADB_CONNECTOR_SUCCESS == mariadbReplaceTable(&mariadbConn,&mariadbRes));
        }
    }

}

TEST_CASE("connectMariadbFailure")
{
    MariadbConnector_t mariadbConn;
    initMariadbConnector(&mariadbConn);
    mariadbConn.connInfo.ip = "192.0.0.1";
    CHECK(MARIADB_CONNECTOR_CONNECT_FAILURE == connectMariadb(&mariadbConn));
}