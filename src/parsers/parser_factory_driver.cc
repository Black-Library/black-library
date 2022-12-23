/**
 * parser_factory_casting.cc
 */

#include <ParserCommon.h>

#include <ParserAO3.h>
#include <ParserFFN.h>
#include <ParserRR.h>

namespace black_library {

namespace core {

namespace parsers {

    std::cout << "first parser\n" << std::endl;

    auto iter = parser_map_.find(RR_PARSER);

    auto parser_got = iter->second;

    std::shared_ptr<RR::ParserRR> rr_parser = std::make_shared<RR::ParserRR>();
    std::shared_ptr<Parser> normal_parser;

    std::cout << "static cast\n" << std::endl;

    normal_parser = std::static_pointer_cast<Parser>(rr_parser);
    normal_parser->Parse();

    std::cout << "dynamic cast\n" << std::endl;

    auto downcasted = std::dynamic_pointer_cast<RR::ParserRR>(normal_parser);
    downcasted->Parse();

    std::cout << "static cast from map\n" << std::endl;

    auto static_cast_from_map = std::static_pointer_cast<Parser>(parser_got);
    static_cast_from_map->Parse();

    // std::cout << "Parser iter: " << GetParserName(iter->first) << std::endl;

    // std::cout << "Parser source url: " << parser_got.GetSourceUrl() << std::endl;
    // std::cout << "Parser type: " << GetParserName(parser_got.GetParserType()) << std::endl;
    // std::cout << "second parser\n" << std::endl;
    // parser_got.Parse();

    std::cout << "end parser test\n" << std::endl;

} // namespace parsers
} // namespace core
} // namespace black_library
