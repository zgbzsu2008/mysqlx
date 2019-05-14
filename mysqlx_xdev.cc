#include <mysqlx/xdevapi.h>
#include <iostream>

using ::std::cout;
using ::std::endl;
using namespace ::mysqlx;

struct
{
  std::string host = "localhost";
  std::string user = "root";
  std::string pwd = "353213";
  std::string db = "mydb";
  unsigned short port = 33060;
} config;

int main(int argc)
{
  try {
    /*
    Session from_uri("mysqlx://" + config.user + ":" + config.pwd + "@" + config.host + ":" +
                     std::to_string(config.port) + "/" + config.db + "?ssl-mode=disabled");

    Session from_options(config.host, config.port, config.user, config.pwd, config.db);

    Session from_option_list(SessionOption::USER, config.host, SessionOption::PWD, config.pwd, SessionOption::HOST,
                             config.host, SessionOption::PORT, config.port, SessionOption::DB, config.db,
                             SessionOption::SSL_MODE, SSLMode::DISABLED);
    */
    Session sess("localhost", 33060, "root", "353213");
    {
      /*
      Session::sql(""): contains `?` placeholders, call `bind()` to define their values
      */
      RowResult res = sess.sql("show variables like 'version'").execute();
      std::stringstream version;

      /*
      RowResult::fetchOne(): Return the current row and move to the next one in the sequence
      Row::get(pos): Get reference to row field at position `pos`
      mysqlx::string Value::get<mysqlx::string>() const
      */
      version << res.fetchOne().get(1).get<string>();
      int major_version;
      version >> major_version;

      if (major_version < 8) {
        cout << "Done!" << endl;
        return 0;
      }
    }

    cout << "Session accepted, creating collection..." << endl;

    /*
    Session::getSchema(): Return an object representing a schema with the given name
    Collection::createCollection(): Represents a collection of documents in a schema
    */
    Schema sch = sess.getSchema("mydb");
    Collection coll = sch.createCollection("mycoll", true);

    cout << "Inserting documents..." << endl;

    coll.remove("true").execute();

    {
      /*
      Result:: Represents a result of an operation that does not return data
      */
      Result add;

      /*
      Collection::add(): Return an operation which adds documents to the collection
      Collection::getGeneratedIds(): Return a list of identifiers of multiple documents added to a collection
      */
      add = coll.add(R"({ "name": "foo", "age": 1 })").execute();
      std::vector<string> ids = add.getGeneratedIds();
      cout << "- added doc with id: " << ids[0] << endl;

      add = coll.add(R"({ "name": "bar", "age": 2, "toys": ["car", "ball"] })").execute();
      ids = add.getGeneratedIds();
      if (ids.size() != 0) {
        cout << "- added doc with id: " << ids[0] << endl;
      } else {
        cout << "- added doc" << endl;
      } 

      add = coll.add(R"({"name": "baz", "age": 3, "date": {"day": 20, "month": "Apr"}})").execute();
      ids = add.getGeneratedIds();
      if (ids.size() != 0) {
        cout << "- added doc with id: " << ids[0] << endl;
      } else {
        cout << "- added doc" << endl;
      } 

      add = coll.add(R"({"_id": "myuuid-1", "name": "foo", "age": 7})").execute();
      ids = add.getGeneratedIds(); // ´ËÊ±ids.size()=0;
      if (ids.size() != 0) { 
        cout << "- added doc with id: " << ids[0] << endl;
      } else {
        cout << "- added doc" << endl;
      } 
    }

    cout << "Fetching documents..." << endl;

    /*
    Collection::find(): Return an operation which finds documents that satisfy given criteria
    */
    DocResult docs = coll.find("age > 1 and name like 'ba%'").execute();

    DbDoc doc = docs.fetchOne();

    for (int i = 0; doc; ++i, doc = docs.fetchOne()) {
      cout << "doc#" << i << ": " << doc << endl;
      
      for (Field fld : doc) { // Field = std::string
        cout << " field `" << fld << "`: " << doc[fld] << endl;
      }

      string name = doc["name"];
      cout << " name: " << name << endl;

      if (doc.hasField("date") && Value::DOCUMENT == doc.fieldType("date")) {
        cout << "- date field" << endl;
        DbDoc date = doc["date"];
        for (Field fld : date) {
          cout << "  date `" << fld << "`: " << date[fld] << endl;
        }
        string month = doc["date"]["month"];
        int day = date["day"];
        cout << "  month: " << month << endl;
        cout << "  day: " << day << endl;
      }

      if (doc.hasField("toys") && Value::ARRAY == doc.fieldType("toys")) {
        cout << "- toys:" << endl;
        for (auto toy : doc["toys"]) {
          cout << "  " << toy << endl;
        }
      }
      cout << endl;
    }
    // sch.dropCollection("mycoll");
    cout << "Done!" << endl;
  } catch (const mysqlx::Error &err) {
    cout << "ERROR: " << err << endl;
    return 1;
  } catch (std::exception &ex) {
    cout << "STD EXCEPTION: " << ex.what() << endl;
    return 1;
  } catch (const char *ex) {
    cout << "EXCEPTION: " << ex << endl;
    return 1;
  }
}