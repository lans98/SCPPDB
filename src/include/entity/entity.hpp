#pragma once

#include <map>
#include <string>
#include <random>
#include <limits>
#include <fstream>
#include <stdexcept>

namespace entity {

    using namespace std;

    using EntityID = unsigned int;
    
    enum EntityType {
        DATABASE,
        FIELD,
        RECORD,
        TABLE,
        UNKNOWN
    };

    /**
     * Generate random unique ID's for each entity
     * database is also a global entity, but it always
     * have the ID 1, and the ID 0 is generic for entities
     * that are waiting for it's ID
     */
    class EntityIDManager {
    private:
        struct EntityInfo { 
            EntityID   parent;
            EntityType idtype;
        };

        struct DiskCell {
            EntityID id;
            EntityID parent;
            EntityType idtype;
        };

        map<EntityID, EntityInfo> used_ids;

        static EntityIDManager* instance;
        EntityIDManager(): used_ids() {}

    public:
        static EntityIDManager& get_instance() {
            if (!instance)
                instance = new EntityIDManager();

            return *instance;
        }

        EntityID generate(EntityID parent_id, EntityType type) {
            random_device rand;
            mt19937_64 gen(rand());
            uniform_int_distribution<EntityID> dist(2, numeric_limits<EntityID>::max());

            EntityID id;
            do {
                id = dist(gen); 
            } while (used_ids.find(id) != used_ids.end());

            used_ids.emplace(id, EntityInfo { .parent = parent_id, .idtype = type });
            return id;
        }

        bool save(EntityID id, EntityID parent_id, EntityType type) {
            auto search = used_ids.find(id);
            if (search != used_ids.end())
                return false;

            used_ids.emplace(id, EntityInfo { .parent = parent_id, .idtype = type });
            return true;
        }
        
        void free(EntityID id)  {
            auto search = used_ids.find(id);
            if (search == used_ids.end())
                return;        

            used_ids.erase(search);
        }

        bool is_valid(EntityID id) {
            auto search = used_ids.find(id);
            return search != used_ids.end();
        }

        bool save_ids(const string& file_name = "") {
            if (file_name == "")        
                return false;

            ofstream file(file_name, ios::binary);
            if (!file)
                return false;

            DiskCell tmp;
            for (auto id: used_ids) {
                tmp.id = id.first;
                tmp.parent = id.second.parent;
                tmp.idtype = id.second.idtype;
                file.write(reinterpret_cast<char*>(&tmp), sizeof(DiskCell));
            }

            return true;
        }

        bool load_ids(const string& file_name = "") {
            if (file_name == "")
                return false;

            ifstream file(file_name, ios::binary);
            if (!file)
                return false;

            auto tmp = new DiskCell();
            while (file) {
                file.read(reinterpret_cast<char*>(tmp), sizeof(DiskCell));
                used_ids.emplace(tmp->id, EntityInfo { .parent = tmp->parent, .idtype = tmp->idtype });
            }

            return true;
        }

        EntityType type_of(EntityID id) {
            if (id == 0U) 
                return EntityType::UNKNOWN;

            auto search = used_ids.find(id);
            if (search == used_ids.end())
                throw runtime_error("Not found ID");
        
            return search->second.idtype;
        }

        EntityID parent_of(EntityID id) {
            if (id == 0U)
                return 0U;

            auto search = used_ids.find(id);
            if (search == used_ids.end())
                throw runtime_error("Not found ID");

            return search->second.parent;
        }
    };

    EntityIDManager* EntityIDManager::instance = nullptr;
    using EntityIDManagerInstance = EntityIDManager&;

    /**
     * Entity class used to identify different
     * kinds of objects in the data base like
     * records, tables or the data base itself
     * because they inherit this type
     */
    class Entity {
    protected:
        EntityID id;

    public:
        Entity(): id(0U) {}
        Entity(EntityType type, optional<EntityID> opt_id, optional<EntityID> opt_parent): 
            id(opt_id.value_or(EntityIDManager::get_instance().generate(opt_parent.value_or(0U), type)))
        { 
            if (opt_id.has_value() && !EntityIDManager::get_instance().save(*opt_id, opt_parent.value_or(0U), type))
                throw runtime_error("Trying to create an entity with an already used id");

            if (!opt_parent.has_value())
                return;

            EntityIDManagerInstance id_manager = EntityIDManager::get_instance();
            switch (type) {
                case EntityType::FIELD:
                    if (id_manager.type_of(*opt_parent) != EntityType::TABLE)
                        throw runtime_error("This entity type is Field, but its parent isn't a Table");
                case EntityType::RECORD:
                    if (id_manager.type_of(*opt_parent) != EntityType::TABLE)
                        throw runtime_error("This entity type is Record, but its parent isn't a Table");
                case EntityType::TABLE:
                    if (id_manager.type_of(*opt_parent) != EntityType::DATABASE)
                        throw runtime_error("This entity type is Table, but its parent isn't the DataBase");
                case EntityType::DATABASE:
                    if (id_manager.type_of(*opt_parent) != EntityType::UNKNOWN)
                        throw runtime_error("This entity type is DataBase, but it has an associated known parent");
                case EntityType::UNKNOWN:
                    if (id_manager.type_of(*opt_parent) != EntityType::UNKNOWN)
                        throw runtime_error("This entity type is Unknown, but it has an associated known parent");
            }
        }

        virtual ~Entity() { EntityIDManager::get_instance().free(id); }
        EntityID get_id() { return id; }        
    };
}
