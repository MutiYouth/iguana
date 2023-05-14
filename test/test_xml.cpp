#include <deque>
#include <iterator>
#include <list>
#include <vector>
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"
#include "iguana/xml_reader.hpp"
#include "iguana/xml_writer.hpp"
#include "rapidxml_print.hpp"
#include <iostream>
#include <optional>

struct simple_t {
  std::vector<int> a;
  char b;
  bool c;
  bool d;
  std::optional<std::string> e;
  bool operator==(const simple_t &other) const {
    if ((b == other.b) && (c == other.c) && (d == other.d) && (e == other.e)) {
      auto t1 = a;
      auto t2 = other.a;
      sort(t1.begin(), t1.end());
      sort(t2.begin(), t2.end());
      return t1 == t2;
    }
    return false;
  }
};
REFLECTION(simple_t, a, b, c, d, e);
TEST_CASE("test simple xml") {
  simple_t simple{{1, 2, 3}, '|', 0, 1};
  std::string str;
  iguana::to_xml_pretty(simple, str);

  simple_t sfrom;
  iguana::from_xml(sfrom, str.data());
  CHECK(sfrom == simple);

  std::string xmlstr = R"(
    <simple_t><a>1</a><a>2</a><a>3</a><b>|</b><c>False</c><d>True</d><e>optional?</e></simple_t>
  )";
  simple_t sfrom2;
  iguana::from_xml(sfrom2, xmlstr.data());
  auto t = sfrom2.a;
  sort(t.begin(), t.end());
  CHECK(t == std::vector{1, 2, 3});
  CHECK(sfrom2.b == '|');
  CHECK(!sfrom2.c);
  CHECK(sfrom2.d);
  CHECK(*sfrom2.e == "optional?");
}

struct nested_t {
  simple_t simple;
  int code;
};
REFLECTION(nested_t, simple, code);
TEST_CASE("test simple nested") {
  std::string str = R"(
    <nested_t>
    <simple><a>1</a><a>2</a><a>3</a><b>|</b><c>False</c><d>True</d><e></e></simple>
    <code>10086</code>
    </nested_t>
  )";
  nested_t nest;
  iguana::from_xml(nest, str.data());
  simple_t res{{1, 2, 3}, '|', 0, 1};
  CHECK(res == nest.simple);
  CHECK(nest.code == 10086);

  std::string toxmlstr;
  iguana::to_xml_pretty(nest, toxmlstr);
  nested_t nest2;
  iguana::from_xml(nest2, toxmlstr.data());
  CHECK(nest2.simple == nest.simple);

  nest2.simple.a = std::vector<int>();
  std::string ss;
  iguana::to_xml(nest2, ss);
}

struct book_t {
  std::string title;
  int edition;
  std::vector<std::string> author;
  std::optional<std::string> description;
  bool operator==(const book_t &other) const {
    if ((title == other.title) && (edition == other.edition) &&
        (description == other.description)) {
      auto t1 = author;
      auto t2 = other.author;
      sort(t1.begin(), t1.end());
      sort(t2.begin(), t2.end());
      return t1 == t2;
    }
    return false;
  }
};

REFLECTION(book_t, title, edition, author, description);
TEST_CASE("test optinal and vector") {
  std::string str = R"(
    <book_t>
      <title>C++ templates</title>
      <edition>2</edition>
      <author>David Vandevoorde</author>
      <author>Nicolai M. Josuttis</author>
      <author>Douglas Gregor</author>
      <description>talking about how to use template</description>
    </book_t> 
  )";
  std::string origin_str = str;
  book_t book;
  iguana::from_xml(book, str.data());
  CHECK(book.title == "C++ templates");
  CHECK(book.author.size() == 3);

  std::vector<std::string> author = {"David Vandevoorde", "Nicolai M. Josuttis",
                                     "Douglas Gregor"};
  sort(author.begin(), author.end());
  auto t = book.author;
  sort(t.begin(), t.end());
  CHECK(t == author);
  CHECK(book.edition == 2);
  CHECK(book.description);
  CHECK(*book.description == "talking about how to use template");

  std::string xml_str;
  iguana::to_xml(book, xml_str);
  book_t newbook;
  iguana::from_xml(newbook, xml_str.data());
  CHECK_MESSAGE(newbook == book, "the newer must be same as the older");
}

struct library_t {
  std::vector<book_t> book;
  int sum;
};
REFLECTION(library_t, book, sum);
TEST_CASE("test nested vector") {
  std::string str = R"(
    <library>
    <book>
      <title>C++ templates</title>
      <edition>2</edition>
      <author>David Vandevoorde</author>
      <author>Nicolai M. Josuttis</author>
      <author>Douglas Gregor</author>
      <description>talking about how to use template</description>
    </book>
    <book>
      <title>C++ primer</title>
      <edition>6</edition>
      <author>Stanley B. Lippman</author>
      <author>Josée Lajoie</author>
      <author>Barbara E. Moo</author>
      <description></description>
    </book>
    <sum>2</sum>
    </library>
  )";
  library_t library;
  iguana::from_xml(library, str.data());
  CHECK(library.sum == 2);
  CHECK(library.book[0].title == "C++ templates");
  CHECK(library.book[1].title == "C++ primer");
  if (library.book[1].title == "C++ primer") {
    CHECK(!library.book[1].description);
    std::vector<std::string> author = {"Stanley B. Lippman", "Josée Lajoie",
                                       "Barbara E. Moo"};
    auto t1 = library.book[1].author;
    CHECK(t1.size() == 3);
    sort(t1.begin(), t1.end());
    sort(author.begin(), author.end());
    CHECK(t1 == author);
  }

  std::string xml_str;
  iguana::to_xml(library, xml_str);
  library_t newlibrary;
  iguana::from_xml(newlibrary, xml_str.data());
  if (newlibrary.book[0].title == library.book[0].title) {
    CHECK(newlibrary.book[0] == library.book[0]);
    CHECK(newlibrary.book[1] == library.book[1]);
  } else {
    CHECK(newlibrary.book[1] == library.book[0]);
    CHECK(newlibrary.book[0] == library.book[1]);
  }
}

struct library_t2 {
  std::optional<std::vector<book_t>> book;
  int sum;
};
REFLECTION(library_t2, book, sum);
TEST_CASE("test optional nested vector") {
  std::string str = R"(
    <library>
    <book>
      <title>C++ templates</title>
      <edition>2</edition>
      <author>David Vandevoorde</author>
      <author>Nicolai M. Josuttis</author>
      <author>Douglas Gregor</author>
      <description>talking about how to use template</description>
    </book>
    <book>
      <title>C++ primer</title>
      <edition>6</edition>
      <author>Stanley B. Lippman</author>
      <author>Josée Lajoie</author>
      <author>Barbara E. Moo</author>
      <description></description>
    </book>
    <sum>2</sum>
    </library>
  )";
  library_t2 library;
  iguana::from_xml(library, str.data());
  CHECK(library.sum == 2);
  auto books = *library.book;
  CHECK(books[0].title == "C++ templates");
  CHECK(books[1].title == "C++ primer");
  if (books[1].title == "C++ primer") {
    CHECK(!books[1].description);
    std::vector<std::string> author = {"Stanley B. Lippman", "Josée Lajoie",
                                       "Barbara E. Moo"};
    auto t1 = books[1].author;
    CHECK(t1.size() == 3);
    sort(t1.begin(), t1.end());
    sort(author.begin(), author.end());
    CHECK(t1 == author);
  }

  std::string xml_str;
  iguana::to_xml(library, xml_str);
  library_t newlibrary;
  iguana::from_xml(newlibrary, xml_str.data());
  if (newlibrary.book[0].title == books[0].title) {
    CHECK(newlibrary.book[0] == books[0]);
    CHECK(newlibrary.book[1] == books[1]);
  } else {
    CHECK(newlibrary.book[1] == books[0]);
    CHECK(newlibrary.book[0] == books[1]);
  }
}

struct book_attr_t {
  std::map<std::string, float> __attr;
  std::string title;
};
REFLECTION(book_attr_t, __attr, title);
TEST_CASE("test attribute with map") {
  std::string str = R"(
    <book_attr_t id="5" pages="392" price="79.9">
      <title>C++ templates</title>
    </book_with_attr_t>
  )";
  book_attr_t b;
  iguana::from_xml(b, str.data());
  CHECK(b.__attr["id"] == 5);
  CHECK(b.__attr["pages"] == 392.0f);
  CHECK(b.__attr["price"] == 79.9f);

  std::string ss;
  iguana::to_xml_pretty(b, ss);
  book_attr_t b2;
  iguana::from_xml(b2, ss.data());
  CHECK(b2.__attr["id"] == 5);
  CHECK(b2.__attr["pages"] == 392.0f);
  CHECK(b2.__attr["price"] == 79.9f);
}

struct book_attr_any_t {
  std::unordered_map<std::string, iguana::any_t> __attr;
  std::string title;
};
REFLECTION(book_attr_any_t, __attr, title);
TEST_CASE("test attribute with any") {
  std::string str = R"(
    <book_attr_any_t id="5" language="en" price="79.9">
      <title>C++ templates</title>
    </book_attr_any_t>
  )";
  book_attr_any_t b;
  iguana::from_xml(b, str.data());
  auto &map = b.__attr;
  CHECK(map["id"].get<int>().first);
  CHECK(map["id"].get<int>().second == 5);
  CHECK(map["language"].get<std::string_view>().first);
  CHECK(map["language"].get<std::string_view>().second == "en");
  CHECK(map["price"].get<float>().first);
  CHECK(map["price"].get<float>().second == 79.9f);

  std::string ss;
  iguana::to_xml(b, ss);
  book_attr_any_t b1;
  iguana::from_xml(b1, ss.data());
  map = b1.__attr;
  CHECK(map["id"].get<int>().first);
  CHECK(map["id"].get<int>().second == 5);
  CHECK(map["language"].get<std::string_view>().first);
  CHECK(map["language"].get<std::string_view>().second == "en");
  CHECK(map["price"].get<float>().first);
  CHECK(map["price"].get<float>().second == 79.9f);
}

struct library_attr_t {
  book_attr_any_t book;
  std::unordered_map<std::string, iguana::any_t> __attr;
};
REFLECTION(library_attr_t, book, __attr);
TEST_CASE("Test nested attribute with any") {
  std::string str = R"(
    <library_attr_t code="102" name="UESTC" time="3.2">
      <book id="5" language="en" price="79.9">
        <title>C++ templates</title>
      </book>
    </library_attr_t>
  )";
  library_attr_t library;
  iguana::from_xml(library, str.data());

  auto map = library.__attr;
  CHECK(map["code"].get<int>().first);
  CHECK(map["code"].get<int>().second == 102);
  CHECK(map["name"].get<std::string>().second == "UESTC");
  CHECK(map["name"].get<std::string_view>().second == "UESTC");
  CHECK(map["time"].get<float>().first);
  CHECK(map["time"].get<float>().second == 3.2f);

  map = library.book.__attr;
  CHECK(map["id"].get<int>().first);
  CHECK(map["id"].get<int>().second == 5);
  CHECK(map["language"].get<std::string_view>().first);
  CHECK(map["language"].get<std::string_view>().second == "en");
  CHECK(map["price"].get<float>().first);
  CHECK(map["price"].get<float>().second == 79.9f);

  std::string ss;
  iguana::to_xml_pretty(library, ss);
  std::cout << ss << std::endl;
  library_attr_t library1;
  iguana::from_xml(library1, ss.data());
  map = library1.__attr;
  CHECK(map["code"].get<int>().first);
  CHECK(map["code"].get<int>().second == 102);
  CHECK(map["name"].get<std::string>().second == "UESTC");
  CHECK(map["name"].get<std::string_view>().second == "UESTC");
  CHECK(map["time"].get<float>().first);
  CHECK(map["time"].get<float>().second == 3.2f);

  map = library1.book.__attr;
  CHECK(map["id"].get<int>().first);
  CHECK(map["id"].get<int>().second == 5);
  CHECK(map["language"].get<std::string_view>().first);
  CHECK(map["language"].get<std::string_view>().second == "en");
  CHECK(map["price"].get<float>().first);
  CHECK(map["price"].get<float>().second == 79.9f);
  CHECK_FALSE(map["language"].get<int>().first); // parse num failed
}

TEST_CASE("test exception") {
  simple_t simple;
  std::string str = R"(
    <simple_t><a>1</a><a>2</a><a>3</a><b>|</b><c>False</c><d>True</d><e></e></
  )";
  CHECK_FALSE(iguana::from_xml(simple, str.data())); // expected >
  std::string str2 = R"(
    <simple_t><a>1</a><a>2</a><a>3</a><b>|</b><c>Flase</c><d>tru</d><e></e></simple_t>
  )";
  CHECK_NOTHROW(iguana::from_xml(simple, str2.data())); // Failed to parse bool
  simple_t simple2{{1, 2, 3}, '|', 0, 1};
  std::string ss = "<<dd>>";
  CHECK_FALSE(iguana::to_xml_pretty(simple2, ss)); // unexpected end of data

  std::string str3 = R"(
    <nested_t>
    <simple><b>|</b><c>False</c><e></e></simple>
    <code></code>
    </nested_t>
  )";
  nested_t nest;
  nest.code = 10;
  iguana::from_xml(nest, str3.data()); // a not found, d not found
  CHECK(nest.simple.a.size() == 0);    // vector is empty
  CHECK(nest.code == 10);              // should never touch it
}

struct item_itunes_t {
  iguana::namespace_t<std::string_view> itunes_author;
  iguana::namespace_t<std::string_view> itunes_subtitle;
  iguana::namespace_t<int> itunes_user;
};
REFLECTION(item_itunes_t, itunes_author, itunes_subtitle, itunes_user);
struct item_t {
  iguana::namespace_t<item_itunes_t> item_itunes;
};
REFLECTION(item_t, item_itunes);
TEST_CASE("test xml namespace") {
  std::string str = R"(
    <item>
      <item:itunes>
        <itunes:author>Jupiter Broadcasting</itunes:author>
        <itunes:subtitle>Linux enthusiasts talk top news stories, subtitle</itunes:subtitle>
        <itunes:user>10086</itunes:user>       
      </item:itunes>
    </item>
  )";
  item_t it;
  iguana::from_xml(it, str.data());
  auto itunes = it.item_itunes.get();
  CHECK(itunes.itunes_author.get() == "Jupiter Broadcasting");
  CHECK(itunes.itunes_user.get() == 10086);

  std::string ss;
  iguana::to_xml(it, ss);
  item_t it2;
  iguana::from_xml(it2, ss.data());
  auto itunes2 = it2.item_itunes.get();
  CHECK(itunes2.itunes_author.get() == "Jupiter Broadcasting");
  CHECK(itunes2.itunes_user.get() == 10086);
}

struct package_t {
  std::pair<std::string, std::unordered_map<std::string, std::string>> version;
  std::pair<std::string, std::unordered_map<std::string, std::string>>
      changelog;
  std::unordered_map<std::string, std::string> __attr;
};
REFLECTION(package_t, version, changelog, __attr);
TEST_CASE("test leafnode attribute") {
  std::string str = R"(
    <package name="apr-util-ldap" arch="x86_64">
      <version epoch="0" ver="1.6.1" rel="6.el8"/>
      <changelog author="Lubo" date="1508932800">new version 1.6.1</changelog>
    </package>
  )";
  package_t package;
  iguana::from_xml(package, str.data());
  using mp = std::unordered_map<std::string, std::string>;
  mp p_attr = {{"name", "apr-util-ldap"}, {"arch", "x86_64"}};
  mp v_attr = {{"epoch", "0"}, {"ver", "1.6.1"}, {"rel", "6.el8"}};
  mp c_attr = {{"author", "Lubo"}, {"date", "1508932800"}};
  CHECK(p_attr == package.__attr);
  CHECK(v_attr == package.version.second);
  CHECK(c_attr == package.changelog.second);
  CHECK(package.changelog.first == "new version 1.6.1");
  CHECK(package.version.first.empty());
  std::string ss;
  iguana::to_xml(package, ss);
  package_t package2;
  iguana::from_xml(package2, ss.data());
  CHECK(p_attr == package2.__attr);
  CHECK(v_attr == package2.version.second);
  CHECK(c_attr == package2.changelog.second);
  CHECK(package2.changelog.first == "new version 1.6.1");
  CHECK(package2.version.first.empty());
}

TEST_CASE("parse error") {
  std::string str = "error xml";
  simple_t p;
  bool r = iguana::from_xml(p, str.data());
  CHECK(!r);
  CHECK(!iguana::get_last_read_err().empty());

  std::string xml_str = R"(
    <book_t>
      <title>C++ templates</title>
      <edition>invalid number</edition>
      <author>David Vandevoorde</author>
      <author>Nicolai M. Josuttis</author>
    </book_t>
  )";
  book_t book;
  r = iguana::from_xml(book, xml_str.data());
  CHECK(!r);
  CHECK(!iguana::get_last_read_err().empty());

  std::string xmlstr = R"(
    <simple_t><a>1</a><a>2</a><a>3</a><b>|</b><c>False</c><d>True</d><e>optional?</e></simple_t>
  )";

  simple_t simple2{{1, 2, 3}, '|', 0, 1};
  std::string ss = "<<dd>>";
  CHECK_FALSE(iguana::to_xml_pretty(simple2, ss)); // unexpected end of data
  CHECK(!iguana::get_last_write_err().empty());
}

TEST_CASE("field not found") {
  std::string xml_str = R"(
    <book_t>
      <title>C++ templates</title>
      <edition2>2</edition2>
      <author>David Vandevoorde</author>
      <author>Nicolai M. Josuttis</author>
    </book_t>
  )";
  book_t book;
  bool r = iguana::from_xml(book, xml_str.data());
  CHECK(r);
}

struct book_with_required {
  std::string title;
  int edition;
  std::vector<std::string> author;
  std::optional<std::string> description;
};
REFLECTION(book_with_required, title, edition, author, description);
REQUIRED(book_with_required, edition);

TEST_CASE("required fields") {
  std::string xml_str = R"(
    <book_with_required>
      <title>C++ templates</title>
      <author>David Vandevoorde</author>
      <author>Nicolai M. Josuttis</author>
    </book_with_required>
  )";
  book_with_required book;
  bool r = iguana::from_xml(book, xml_str.data());
  CHECK(!r);
  std::cout << iguana::get_last_read_err() << "\n";
  CHECK(!iguana::get_last_read_err().empty());
}

struct description_t {
  iguana::cdata_t cdata;
};
REFLECTION(description_t, cdata);
struct optionc_t {
  std::optional<iguana::cdata_t> cdata;
};
REFLECTION(optionc_t, cdata);
struct node_t {
  std::string title;
  description_t description;
  optionc_t option;
  std::vector<iguana::cdata_t> cdata;
};
REFLECTION(node_t, title, description, option, cdata);
TEST_CASE("test cdata node") {
  std::string str = R"(
    <node_t>
      <title>what's the cdata</title>
      <description>
        <a>what's the cdata</a>
        <![CDATA[<p>nest cdata node</p>]]>
      </description>
      <option>
        <![CDATA[<p>this is a option cdata</p>]]>
      </option>
      <![CDATA[<p>this is a  cdata node</p>]]>
      <![CDATA[<p>this is a  cdata node</p>]]>
    </node_t>
  )";
  node_t node;
  iguana::from_xml(node, str.data());
  CHECK(node.title == "what's the cdata");
  CHECK(node.description.cdata.get() == "<p>nest cdata node</p>");
  CHECK((*(node.option.cdata)).get() == "<p>this is a option cdata</p>");
  CHECK(node.cdata[0].get() == "<p>this is a  cdata node</p>");
  CHECK(node.cdata[1].get() == "<p>this is a  cdata node</p>");
  std::string ss;
  iguana::to_xml(node, ss);
  node_t node1;
  iguana::from_xml(node1, ss.data());
  CHECK(node1.title == "what's the cdata");
  CHECK(node1.description.cdata.get() == "<p>nest cdata node</p>");
  CHECK((*(node1.option.cdata)).get() == "<p>this is a option cdata</p>");
  CHECK(node1.cdata[0].get() == "<p>this is a  cdata node</p>");
  CHECK(node1.cdata[1].get() == "<p>this is a  cdata node</p>");

  std::string str2 = R"(
    <description>
    </description>
  )";
  description_t dscrp;
  iguana::from_xml(dscrp, str2.data());
  CHECK(dscrp.cdata.get().empty());
}

// doctest comments
// 'function' : must be 'attribute' - see issue #182
DOCTEST_MSVC_SUPPRESS_WARNING_WITH_PUSH(4007)
int main(int argc, char **argv) { return doctest::Context(argc, argv).run(); }
DOCTEST_MSVC_SUPPRESS_WARNING_POP
