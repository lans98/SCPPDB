#pragma once

#include <string>
#include <unordered_map>

#include <core/data_types.hpp>
#include <entity/entity.hpp>
#include <concurrency/lock_manager.hpp>

namespace concurrency_manager {

    using namespace std;
    using namespace data_types;
    using namespace lock_manager;

    class ConcurrencyManager {
    private:
        LockManager m_lock_manager;

    public:

    };

}
