#include <fstream>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>
#pragma warning(push)
// conversion from std::streamsize to int, possible loss of data
#pragma warning(disable : 4244)
#include <boost/iostreams/copy.hpp>
#pragma warning(pop)
#include <boost/iostreams/filter/bzip2.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/iostreams/filter/zlib.hpp>

int main(int argc, char** argv)
{
  namespace bfs = boost::filesystem;
  namespace bio = boost::iostreams;
  bfs::path exepath = bfs::path(std::string(argv[0]));
  if (argc != 2)
  {
    std::cerr << "usage: " << exepath.filename().string() << " file.[bz2|gz|Z]" << std::endl;
    return 1;
  }
  try
  {
    bfs::path filepath = bfs::path(std::string(argv[1]));
    if (!bfs::exists(filepath))
    {
      std::cerr << filepath.string() << ": doesn't exist" << std::endl;
      return 1;
    }
    bio::filtering_streambuf<bio::input> in;
    std::string ext = filepath.extension().string();
    if (ext.compare(".bz2") == 0)
      in.push(bio::bzip2_decompressor());
    else if (ext.compare(".gz") == 0)
      in.push(bio::gzip_decompressor());
    else if (ext.compare(".Z") == 0)
      in.push(bio::zlib_decompressor());
    else
    {
      std::cerr << filepath.filename().string() << ": unsupported extension (must be .[bz2|gz|Z])" << std::endl;
      return 1;
    }
    std::ifstream file(argv[1], std::ios_base::in | std::ios_base::binary);
    in.push(file);
    bio::copy(in, std::cout);
  }
  //catch (const bio::bzip2_error& e)
  catch (const bio::zlib_error& e)
  {
    int err = e.error();
    if (err == bio::zlib::buf_error)
      std::cerr << "zlib buffer error" << std::endl;
    else if (err == bio::zlib::data_error)
      std::cerr << "zlib data error" << std::endl;
    else if (err == bio::zlib::mem_error)
      std::cerr << "zlib memory error" << std::endl;
    else if (err == bio::zlib::stream_error)
      std::cerr << "zlib stream error" << std::endl;
    else if (err == bio::zlib::version_error)
      std::cerr << "zlib version error" << std::endl;
    else
      std::cerr << "zlib unknown error" << std::endl;
    return 1;
  }
  catch (const std::exception& e)
  {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
