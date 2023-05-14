#include "xml_bench.hpp"

class ScopedTimer {
public:
  ScopedTimer(const char *name)
      : m_name(name), m_beg(std::chrono::high_resolution_clock::now()) {}
  ScopedTimer(const char *name, uint64_t &ns) : ScopedTimer(name) {
    m_ns = &ns;
  }
  ~ScopedTimer() {
    auto end = std::chrono::high_resolution_clock::now();
    auto dur =
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - m_beg);
    if (m_ns)
      *m_ns = dur.count();
    else
      std::cout << std::left << std::setw(45) << m_name << " : " << std::right
                << std::setw(12) << dur.count() << " ns\n";
  }

private:
  const char *m_name;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_beg;
  uint64_t *m_ns = nullptr;
};

std::string xml_file_content(const std::string &filename) {
  std::error_code ec;
  uint64_t size = std::filesystem::file_size(filename, ec);
  if (ec) {
    throw std::runtime_error("file size error " + ec.message());
  }
  if (size == 0) {
    throw std::runtime_error("empty file");
  }
  std::string content;
  content.resize(size);
  std::ifstream file(filename, std::ios::binary);
  file.read(content.data(), content.size());
  return content;
}

void test_deserialize() {
  int iterations = 1000;

  std::cout << "============ deserialize rpm_filelists.xml  ===============\n";
  std::string xmlfilelist = xml_file_content("../data/rpm_filelists.xml");
  {

    ScopedTimer timer("test deserialize rpm_filelists.xml");
    for (int i = 0; i < iterations; ++i) {
      filelists_t filelist;
      iguana::from_xml<rapidxml::parse_fastest>(filelist, xmlfilelist.data());
    }
  }
  std::cout << "============ deserialize sample_rss.xml    ===============\n";

  std::string xmlrss = xml_file_content("../data/sample_rss.xml");
  {

    ScopedTimer timer("test deserialize sample_rss.xml");
    for (int i = 0; i < iterations; ++i) {
      rss_t rss;
      iguana::from_xml<rapidxml::parse_fastest>(rss, xmlrss.data());
      // assert(rss.channel.item.size() == 99);
    }
  }
}

int main() { test_deserialize(); }
