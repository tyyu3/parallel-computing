#ifndef AUXILLARY_HPP
#define AUXILLARY_HPP

#include <boost/program_options.hpp>

#include <iostream>
namespace aux
{
    namespace po =  boost::program_options;

    std::vector<po::option> ignore_numbers(std::vector<std::string>& args)
    {
        std::vector<po::option> result;
        int pos = 0;
        while(!args.empty()) {
            const auto& arg = args[0];
            double num;
            if(boost::conversion::try_lexical_convert(arg, num)) {
                result.push_back(po::option());
                po::option& opt = result.back();

                opt.position_key = pos++;
                opt.value.push_back(arg);
                opt.original_tokens.push_back(arg);

                args.erase(args.begin());
            } else {
                break;
            }
        }

        return result;
    }

    std::pair<std::int64_t, bool> parse_cmd(int argc, char* argv[])
    {
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help", "produce help message")
            ("to-abs", po::value<std::int64_t>()->required(), "the value to take abs() of");
        po::variables_map vm;
        po::positional_options_description p;
        p.add("to-abs", 1);
        try
        {
            po::store(po::command_line_parser(argc, argv).options(desc).extra_style_parser(&ignore_numbers).positional(p).run(), vm);
            if (!vm["help"].empty())
            {
                std::cout << desc << '\n' << "Usage:" << argv[0] << " <to-abs>"<< '\n';
                return {0, false};
            }
            po::notify(vm);
        }
        catch (const po::error& error)
        {
            std::cerr << "Error while parsing command-line arguments: "
                      << error.what() << "\nPlease use --help to see help message\n";
        }
        return {vm["to-abs"].as<std::int64_t>(), true};
    }

    struct Timings
    {
        std::size_t iterations;
        double ticks_per_iter;
        double ns_per_iter;
    };

}
#endif // AUXILLARY_HPP
