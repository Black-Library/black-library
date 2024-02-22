/**
 * BlackLibraryRESTAPI.h
 */

#ifndef __BLACK_LIBRARY_CORE_REST_API_BLACKLIBRARYRESTAPI_H__
#define __BLACK_LIBRARY_CORE_REST_API_BLACKLIBRARYRESTAPI_H__

namespace black_library {

namespace core {

namespace rest {

class BlackLibraryRESTAPI {
public:
private:
    std::shared_ptr<Pistache::Http::Endpoint> endpoint_;
    Pistache::Rest::Router rest_router_;
    Pistache::Address address_;
    uint16_t port_number_;
    uint16_t num_threads_;
}

} // namespace rest
} // namespace core
} // namespace black_library

#endif
