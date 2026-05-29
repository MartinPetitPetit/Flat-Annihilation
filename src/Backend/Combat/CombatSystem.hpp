
#include <memory>
#include <vector>

#include "../Map/Map.hpp"
#include "../Player/Player.hpp"
#include "../Unit/Unit.hpp"
#include "../../Frontend/Sound/Sound.hpp"

class CombatSystem
{
public:
    static void update(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        int tickRate,
        Sound* sound = nullptr
    );

    static bool removeDeadEntities(
        MAP& map,
        std::vector<std::unique_ptr<Player>>& players,
        std::vector<std::unique_ptr<Unit>>& units,
        Sound* sound = nullptr
    );
};