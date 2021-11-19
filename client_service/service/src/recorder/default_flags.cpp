#include "recorder/default_flags.h"
#include "recorder/game_recorder.h"
#include <VersionHelpers.h>

namespace service::recorder {

int getDefaultRecordingFlagsForGame(shared::EGame game) {
    // Do any games actually record OK with GDI??? I'm starting to doubt it.
    switch (game) {
        case shared::EGame::Aimlab:
        case shared::EGame::CSGO:
        case shared::EGame::Hearthstone:
        case shared::EGame::LeagueOfLegends:
        case shared::EGame::TFT:
        case shared::EGame::WoW:
        case shared::EGame::WowVanilla:
        case shared::EGame::WowTbc:
        case shared::EGame::Valorant:
        return service::recorder::FLAG_DXGI_RECORDING |
            (IsWindows10OrGreater() ? service::recorder::FLAG_WGC_RECORDING : service::recorder::FLAG_GDI_RECORDING);
    }
    return FLAG_ALL_RECORDING;
}

}