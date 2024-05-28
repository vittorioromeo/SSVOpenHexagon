// Copyright (c) 2013-2020 Vittorio Romeo
// License: Academic Free License ("AFL") v. 3.0
// AFL License page: https://opensource.org/licenses/AFL-3.0

#include "SSVOpenHexagon/Global/Config.hpp"

#include "SSVOpenHexagon/Global/UtilsJson.hpp"
#include "SSVOpenHexagon/Utils/String.hpp"
#include "SSVOpenHexagon/Utils/Casts.hpp"
#include "SSVOpenHexagon/Core/Joystick.hpp"

#include "SSVOpenHexagon/SSVUtilsJson/SSVUtilsJson.hpp"
#include "SSVOpenHexagon/SSVUtilsJson/LinkedValue/LinkedValue.hpp"

#include <SSVStart/Utils/Input.hpp>
#include <SSVStart/Input/Input.hpp>
#include <SSVStart/GameSystem/GameWindow.hpp>

#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/Joystick.hpp>

#include <SFML/System/Vector2.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

[[nodiscard]] static const std::vector<std::string>&
defaultServerLevelWhitelist()
{
    static const std::vector<std::string> result{
        // Vittorio Romeo - Cube
        "ohvrvanilla_vittorio_romeo_cube_1_apeirogon_m_0.35",
        "ohvrvanilla_vittorio_romeo_cube_1_apeirogon_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_apeirogon_m_1.6",
        "ohvrvanilla_vittorio_romeo_cube_1_commando_m_0.5",
        "ohvrvanilla_vittorio_romeo_cube_1_commando_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_commando_m_1.5",
        "ohvrvanilla_vittorio_romeo_cube_1_euclideanpc_m_0.5",
        "ohvrvanilla_vittorio_romeo_cube_1_euclideanpc_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_euclideanpc_m_1.8",
        "ohvrvanilla_vittorio_romeo_cube_1_flatteringshape_m_0.5",
        "ohvrvanilla_vittorio_romeo_cube_1_flatteringshape_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_flatteringshape_m_3",
        "ohvrvanilla_vittorio_romeo_cube_1_flatteringshape_m_4",
        "ohvrvanilla_vittorio_romeo_cube_1_goldenratio_m_0.5",
        "ohvrvanilla_vittorio_romeo_cube_1_goldenratio_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_goldenratio_m_2",
        "ohvrvanilla_vittorio_romeo_cube_1_labyrinth_m_0.5",
        "ohvrvanilla_vittorio_romeo_cube_1_labyrinth_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_labyrinth_m_1.5",
        "ohvrvanilla_vittorio_romeo_cube_1_pi_m_0.5",
        "ohvrvanilla_vittorio_romeo_cube_1_pi_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_pi_m_1.8",
        "ohvrvanilla_vittorio_romeo_cube_1_pointless_m_0.5",
        "ohvrvanilla_vittorio_romeo_cube_1_pointless_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_pointless_m_3",
        "ohvrvanilla_vittorio_romeo_cube_1_pointless_m_5",
        "ohvrvanilla_vittorio_romeo_cube_1_seconddimension_m_0.6",
        "ohvrvanilla_vittorio_romeo_cube_1_seconddimension_m_1",
        "ohvrvanilla_vittorio_romeo_cube_1_seconddimension_m_1.8",
        "ohvrvanilla_vittorio_romeo_cube_1_seconddimension_m_2.2",

        // Vittorio Romeo - Orthoplex
        "ohvrvanilla_vittorio_romeo_orthoplex_1_bipolarity_m_0.5",
        "ohvrvanilla_vittorio_romeo_orthoplex_1_bipolarity_m_1",
        "ohvrvanilla_vittorio_romeo_orthoplex_1_bipolarity_m_1.8",

        // Vittorio Romeo - Hypercube
        "ohvrvanilla_vittorio_romeo_hypercube_1_acceleradiant_m_0.85",
        "ohvrvanilla_vittorio_romeo_hypercube_1_acceleradiant_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_acceleradiant_m_1.8",
        "ohvrvanilla_vittorio_romeo_hypercube_1_acceleradiant_m_2.4",
        "ohvrvanilla_vittorio_romeo_hypercube_1_centrifugal_m_0.6",
        "ohvrvanilla_vittorio_romeo_hypercube_1_centrifugal_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_centrifugal_m_1.6",
        "ohvrvanilla_vittorio_romeo_hypercube_1_disc-o_m_0.5",
        "ohvrvanilla_vittorio_romeo_hypercube_1_disc-o_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_disc-o_m_2",
        "ohvrvanilla_vittorio_romeo_hypercube_1_disc-o_m_2.8",
        "ohvrvanilla_vittorio_romeo_hypercube_1_g-force_m_0.6",
        "ohvrvanilla_vittorio_romeo_hypercube_1_g-force_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_g-force_m_1.6",
        "ohvrvanilla_vittorio_romeo_hypercube_1_g-force_m_2.8",
        "ohvrvanilla_vittorio_romeo_hypercube_1_incongruence_m_0.5",
        "ohvrvanilla_vittorio_romeo_hypercube_1_incongruence_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_incongruence_m_1.8",
        "ohvrvanilla_vittorio_romeo_hypercube_1_massacre_m_0.5",
        "ohvrvanilla_vittorio_romeo_hypercube_1_massacre_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_massacre_m_1.6",
        "ohvrvanilla_vittorio_romeo_hypercube_1_polyhedrug_m_0.75",
        "ohvrvanilla_vittorio_romeo_hypercube_1_polyhedrug_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_polyhedrug_m_1.8",
        "ohvrvanilla_vittorio_romeo_hypercube_1_reppaws_m_0.5",
        "ohvrvanilla_vittorio_romeo_hypercube_1_reppaws_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_reppaws_m_1.8",
        "ohvrvanilla_vittorio_romeo_hypercube_1_slither_m_0.5",
        "ohvrvanilla_vittorio_romeo_hypercube_1_slither_m_1",
        "ohvrvanilla_vittorio_romeo_hypercube_1_slither_m_1.5",
        "ohvrvanilla_vittorio_romeo_hypercube_1_slither_m_2",

        // Vipre - Vanity [Steam Workshop]
        "vanitylevels_Vipre_Vanity_10006_haunted_m_1",
        "vanitylevels_Vipre_Vanity_10006_haunted_m_1.001",
        "vanitylevels_Vipre_Vanity_10006_mantra_m_1",
        "vanitylevels_Vipre_Vanity_10006_menace_m_1",
        "vanitylevels_Vipre_Vanity_10006_menace_m_1.001",
        "vanitylevels_Vipre_Vanity_10006_narcotics_m_1",
        "vanitylevels_Vipre_Vanity_10006_narcotics_m_1.001",
        "vanitylevels_Vipre_Vanity_10006_rocketscience_m_1",
        "vanitylevels_Vipre_Vanity_10006_rocketscience_m_1.001",
        "vanitylevels_Vipre_Vanity_10006_singlepulseremixv2remix_m_1",
        "vanitylevels_Vipre_Vanity_10006_squarepractice_m_1",
        "vanitylevels_Vipre_Vanity_10006_squarepractice_m_1.003",
        "vanitylevels_Vipre_Vanity_10006_squarepractice_m_1.005",
        "vanitylevels_Vipre_Vanity_10006_technology_m_1",
        "vanitylevels_Vipre_Vanity_10006_technology_m_1.001",
        "vanitylevels_Vipre_Vanity_10006_troglodyte_m_1",
        "vanitylevels_Vipre_Vanity_10006_troglodyte_m_1.001",
        "vanitylevels_Vipre_Vanity_10006_troglodyte_m_1.002",

        // clang-format off
        // Morxemplum - Rotationality Remastered [Steam Workshop]
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_dragonMayhem_m_1",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_dragonMayhem_m_2",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_dragonMayhem_m_3",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_dragonMayhem_m_5",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_dragonMayhem_m_6.66",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_skyFasion_m_1",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_skyFasion_m_2",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_skyFasion_m_3",
        "rotationality_SkyMidnight_Rotationality_(Remastered)_2_skyFasion_m_5",
        // clang-format on

        // Syyrion - Travel [Steam Workshop]
        "1D2B-E31E-EC19_Syyrion_Travel_13103_ti84_m_0.8",
        "1D2B-E31E-EC19_Syyrion_Travel_13103_ti84_m_1",
        "1D2B-E31E-EC19_Syyrion_Travel_13103_ti84_m_1.2",
        "1D2B-E31E-EC19_Syyrion_Travel_13103_ti84_m_1.4",
        "1D2B-E31E-EC19_Syyrion_Travel_13103_spl_m_1",

        // NDigger - Delta [Steam Workshop]
        "ndeltalevels_NDigger_delta_1_escape_m_1",
        "ndeltalevels_NDigger_delta_1_escape_m_1.001",
        "ndeltalevels_NDigger_delta_1_fusionZ_m_1",
        "ndeltalevels_NDigger_delta_1_fusionZ_m_1.001",
        "ndeltalevels_NDigger_delta_1_fusionZ_m_1.002",
        "ndeltalevels_NDigger_delta_1_hyperbounce_m_1",
        "ndeltalevels_NDigger_delta_1_hyperbounce_m_1.001",
        "ndeltalevels_NDigger_delta_1_matrix_m_1",
        "ndeltalevels_NDigger_delta_1_matrix_m_1.001",
        "ndeltalevels_NDigger_delta_1_nocturnal_m_1",
        "ndeltalevels_NDigger_delta_1_nocturnal_m_1.001",
        "ndeltalevels_NDigger_delta_1_radioactive_m_1",
        "ndeltalevels_NDigger_delta_1_radioactive_m_1.001",
        "ndeltalevels_NDigger_delta_1_snowstorm_m_1",
        "ndeltalevels_NDigger_delta_1_snowstorm_m_1.001",
        "ndeltalevels_NDigger_delta_1_synergic_m_1",
        "ndeltalevels_NDigger_delta_1_synergic_m_1.001",
        "ndeltalevels_NDigger_delta_1_underworld_m_1",
        "ndeltalevels_NDigger_delta_1_underworld_m_1.001",
        "ndeltalevels_NDigger_delta_1_viper_m_1",
        "ndeltalevels_NDigger_delta_1_viper_m_1.001",

        // Hunter & others - Open Hexagon League 2 - Pack [Steam Workshop]
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Automaton_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Automaton_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Ezekiel_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Ezekiel_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Frostbite_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Frostbite_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Insight_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Insight_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Neurodance_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Neurodance_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Partygoer_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Partygoer_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Technicolor_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Technicolor_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Toxine_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Toxine_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Vault_m_1",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_Vault_m_1.01",
        "ohl2pack_Hunter_&_Maniac_Open_Hexagon_League_2_1_VaultFixed_m_1.02",

        // Hunter & others - Open Hexagon League 3 - Pack [Steam Workshop]
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Blitz_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Blitz_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Bloodlines_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Bloodlines_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Cybereye_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Cybereye_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Cyclone_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Cyclone_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Cyclone_m_1.015",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Darkland_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Darkland_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Evirex_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Evirex_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Onyx_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Onyx_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Reactive_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Reactive_m_1.01",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Wilderness_m_1",
        "ohl3pack_Hunter_&_Maniac_Open_Hexagon_League_3_1_Wilderness_m_1.01",

        // clang-format off
        // Hunter & others - Open Hexagon League 3 - Pack [Steam Workshop]
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Catharsis_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Catharsis_m_1.01",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Heartbeat_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Heartbeat_m_1.01",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Joululumi_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Joululumi_m_1.001",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_JoululumiNormal_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_JoululumiNormal_m_1.001",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Killerbox_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Killerbox_m_1.01",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Luminara_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Luminara_m_1.01",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Sharpshooter_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Sharpshooter_m_1.01",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Signal_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_Signal_m_1.01",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_eclipse_m_1",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_eclipse_m_1.01",
        "ohl4pack_Hunter_&_Fractal_&_zX_Open_Hexagon_League_4_1_teraflect_m_1"
        // clang-format on

        //
    };

    return result;
}

using uint = unsigned int;
using ushort = unsigned short;

using trig = ssvs::Input::Trigger;

using k = sf::Keyboard::Key;
using m = sf::Mouse::Button;
using cmb = ssvs::Input::Combo;

using kil = std::initializer_list<sf::Keyboard::Key>;
using mil = std::initializer_list<sf::Mouse::Button>;
using cil = std::initializer_list<cmb>;

#define X_LINKEDVALUES_BINDS_JOYSTICK                    \
    X(joystickSelect, uint, "j_select", 1)               \
    X(joystickExit, uint, "j_exit", 2)                   \
    X(joystickFocus, uint, "j_focus", 4)                 \
    X(joystickSwap, uint, "j_swap", 5)                   \
    X(joystickForceRestart, uint, "j_force_restart", 3)  \
    X(joystickRestart, uint, "j_restart", 0)             \
    X(joystickReplay, uint, "j_replay", 6)               \
    X(joystickScreenshot, uint, "j_screenshot", 7)       \
    X(joystickNextPack, uint, "j_next", 11)              \
    X(joystickPreviousPack, uint, "j_previous", 10)      \
    X(joystickAddToFavorites, uint, "j_add_favorite", 8) \
    X(joystickFavoritesMenu, uint, "j_favorite_menu", 9)

#define X_LINKEDVALUES_BINDS_TRIGGERS                                      \
    X(triggerRotateCCW, trig, "t_rotate_ccw",                              \
        cil{cmb{{k::A}}, cmb{{k::Left}}, cmb{kil{}, mil{m::Left}}})        \
    X(triggerRotateCW, trig, "t_rotate_cw",                                \
        cil{cmb{{k::D}}, cmb{{k::Right}}, cmb{kil{}, mil{m::Right}}})      \
    X(triggerFocus, trig, "t_focus",                                       \
        cil{cmb{{k::LShift}}, cmb{kil{}, mil{m::Extra1}}})                 \
    X(triggerSelect, trig, "t_select",                                     \
        cil{cmb{{k::Space}}, cmb{kil{}, mil{m::Middle}}})                  \
    X(triggerExit, trig, "t_exit",                                         \
        cil{cmb{{k::T}}, cmb{kil{}, mil{m::Extra2}}})                      \
    X(triggerForceRestart, trig, "t_force_restart",                        \
        cil{cmb{{k::Up}}, cmb{{k::R}}})                                    \
    X(triggerRestart, trig, "t_restart",                                   \
        cil{cmb{{k::Space}}, cmb{{k::Enter}}, cmb{kil{}, mil{m::Middle}}}) \
    X(triggerReplay, trig, "t_replay", cil{cmb{{k::Y}}})                   \
    X(triggerScreenshot, trig, "t_screenshot", cil{cmb{{k::F12}}})         \
    X(triggerSwap, trig, "t_swap",                                         \
        cil{cmb{{k::Space}}, cmb{kil{}, mil{m::Middle}}})                  \
    X(triggerUp, trig, "t_up", cil{cmb{{k::W}}})                           \
    X(triggerDown, trig, "t_down", cil{cmb{{k::S}}})                       \
    X(triggerNextPack, trig, "t_next", cil{cmb{{k::PageDown}}})            \
    X(triggerPreviousPack, trig, "t_previous", cil{cmb{{k::PageUp}}})      \
    X(triggerLuaConsole, trig, "t_lua_console", cil{cmb{{k::F1}}})         \
    X(triggerPause, trig, "t_pause", cil{cmb{{k::F2}}})

#define X_LINKEDVALUES_BINDS      \
    X_LINKEDVALUES_BINDS_JOYSTICK \
    X_LINKEDVALUES_BINDS_TRIGGERS

#define X_LINKEDVALUES                                                     \
    X(official, bool, "official", true)                                    \
    X(noPulse, bool, "no_pulse", false)                                    \
    X(noRotation, bool, "no_rotation", false)                              \
    X(noBackground, bool, "no_background", false)                          \
    X(noSound, bool, "no_sound", false)                                    \
    X(noMusic, bool, "no_music", false)                                    \
    X(blackAndWhite, bool, "black_and_white", false)                       \
    X(pulseEnabled, bool, "pulse_enabled", true)                           \
    X(_3DEnabled, bool, "3D_enabled", true)                                \
    X(shadersEnabled, bool, "shaders_enabled", true)                       \
    X(_3DMultiplier, float, "3D_multiplier", 1.f)                          \
    X(_3DMaxDepth, uint, "3D_max_depth", 100)                              \
    X(invincible, bool, "invincible", false)                               \
    X(autoRestart, bool, "auto_restart", false)                            \
    X(soundVolume, float, "sound_volume", 100.f)                           \
    X(musicVolume, float, "music_volume", 100.f)                           \
    X(flashEnabled, bool, "flash_enabled", true)                           \
    X(zoomFactor, float, "zoom_factor", 1.27f)                             \
    X(pixelMultiplier, int, "pixel_multiplier", 1)                         \
    X(playerSpeed, float, "player_speed", 9.45f)                           \
    X(playerFocusSpeed, float, "player_focus_speed", 4.625f)               \
    X(playerSize, float, "player_size", 7.3f)                              \
    X(limitFPS, bool, "limit_fps", true)                                   \
    X(vsync, bool, "vsync", false)                                         \
    X(autoZoomFactor, bool, "auto_zoom_factor", true)                      \
    X(fullscreen, bool, "fullscreen", false)                               \
    X(windowedAutoResolution, bool, "windowed_auto_resolution", false)     \
    X(fullscreenAutoResolution, bool, "fullscreen_auto_resolution", false) \
    X(fullscreenWidth, uint, "fullscreen_width", 1920)                     \
    X(fullscreenHeight, uint, "fullscreen_height", 1080)                   \
    X(windowedWidth, uint, "windowed_width", 800)                          \
    X(windowedHeight, uint, "windowed_height", 600)                        \
    X(showMessages, bool, "show_messages", true)                           \
    X(debug, bool, "debug", false)                                         \
    X(beatPulse, bool, "beatpulse_enabled", true)                          \
    X(showTrackedVariables, bool, "show_tracked_variables", true)          \
    X(musicSpeedDMSync, bool, "music_speed_dm_sync", true)                 \
    X(maxFPS, uint, "max_fps", 200)                                        \
    X(antialiasingLevel, uint, "antialiasing_level", 4)                    \
    X(showFPS, bool, "show_fps", false)                                    \
    X(musicSpeedMult, float, "music_speed_mult", 1.0f)                     \
    X(drawTextOutlines, bool, "draw_text_outlines", true)                  \
    X(darkenUnevenBackgroundChunk, bool, "darken_uneven_background_chunk", \
        true)                                                              \
    X(rotateToStart, bool, "rotate_to_start", false)                       \
    X(joystickDeadzone, float, "joystick_deadzone", 5.0f)                  \
    X(textPadding, float, "text_padding", 8.0f)                            \
    X(textScaling, float, "text_scaling", 1.0f)                            \
    X(timescale, float, "timescale", 1.0f)                                 \
    X(showKeyIcons, bool, "show_key_icons", false)                         \
    X(keyIconsScale, float, "key_icons_scale", 0.75f)                      \
    X(firstTimePlaying, bool, "first_time_playing", true)                  \
    X(showLevelInfo, bool, "show_level_info", false)                       \
    X(showTimer, bool, "show_timer", true)                                 \
    X(showStatusText, bool, "show_status_text", true)                      \
    X(serverIp, std::string, "server_ip", "139.162.199.162")               \
    X(serverPort, ushort, "server_port", 50505)                            \
    X(serverControlPort, ushort, "server_control_port", 50506)             \
    X(serverLevelWhitelist, std::vector<std::string>,                      \
        "server_level_whitelist", defaultServerLevelWhitelist())           \
    X(saveLastLoginUsername, bool, "save_last_login_username", true)       \
    X(lastLoginUsername, std::string, "last_login_username", "")           \
    X(showLoginAtStartup, bool, "show_login_at_startup", false)            \
    X(cameraShakeMultiplier, float, "camera_shake_multiplier", 1.f)        \
    X(angleTiltIntensity, float, "angle_tilt_intensity", 1.f)              \
    X(showPlayerTrail, bool, "show_player_trail", true)                    \
    X(playerTrailAlpha, uint, "player_trail_alpha", 35)                    \
    X(playerTrailScale, float, "player_trail_scale", 0.9f)                 \
    X(playerTrailDecay, float, "player_trail_decay", 3.0f)                 \
    X(playerTrailHasSwapColor, bool, "player_trail_has_swap_color", true)  \
    X(showSwapParticles, bool, "show_swap_particles", true)                \
    X(playSwapReadySound, bool, "play_swap_ready_sound", true)             \
    X(showSwapBlinkingEffect, bool, "show_swap_blinking_effect", true)     \
    X(useLuaFileCache, bool, "use_lua_file_cache", false)                  \
    X(disableGameRendering, bool, "disable_game_rendering", false)         \
    X_LINKEDVALUES_BINDS

// TODO: enable cache on server

namespace hg::Config {

[[nodiscard]] static ssvuj::Obj& root() noexcept
{
    static ssvuj::Obj res = []
    {
        if (ssvufs::Path{"config.json"}.isFile())
        {
            ssvu::lo("hg::Config::root()")
                << "User-defined `config.json` file found\n";

            return ssvuj::getFromFile("config.json");
        }

        ssvu::lo("hg::Config::root()")
            << "No suitable config file found, using defaults\n";

        return ssvuj::Obj{};
    }();

    return res;
}

#if defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wbraced-scalar-init"
#endif

#define X(name, type, key, ...)                                               \
    [[nodiscard]] static auto& name() noexcept                                \
    {                                                                         \
        static auto res = ::ssvuj::LinkedValue<type>(key, type{__VA_ARGS__}); \
        return res;                                                           \
    }
X_LINKEDVALUES
#undef X

#if defined(__clang__)
#pragma GCC diagnostic pop
#endif

static void fixupMissingTriggers()
{
    const auto doIt = [](ssvs::Input::Trigger& trig)
    {
        auto& combos = trig.getCombos();

        if (!combos.empty())
        {
            return;
        }

        combos.resize(4);

        for (ssvs::Input::Combo& c : combos)
        {
            c.clearBind(); // mark as unbound
        }
    };

#define X(name, type, key, ...) doIt(name());
    X_LINKEDVALUES_BINDS_TRIGGERS
#undef X
}

static void syncAllFromObj()
{
#define X(name, type, key, ...) name().syncFrom(root());
    X_LINKEDVALUES
#undef X

    fixupMissingTriggers();
}

static void syncAllToObj()
{
#define X(name, type, key, ...) name().syncTo(root());
    X_LINKEDVALUES
#undef X
}

static void resetAllFromDefault()
{
#define X(name, type, key, ...) name().resetToDefault();
    X_LINKEDVALUES
#undef X
}

static void resetBindsFromDefault()
{
#define X(name, type, key, ...) name().resetToDefault();
    X_LINKEDVALUES_BINDS
#undef X
}

#undef X_LINKEDVALUES
#undef X_LINKEDVALUES_BINDS
#undef X_LINKEDVALUES_BINDS_TRIGGERS
#undef X_LINKEDVALUES_BINDS_JOYSTICK

float sizeX{1500}, sizeY{1500};
constexpr float defaultSpawnDistance{1600};
std::string uneligibilityReason;

static void applyAutoWindowedResolution()
{
    auto d = sf::VideoMode::getDesktopMode();
    windowedWidth() = d.size.x;
    windowedHeight() = d.size.y;
}

static void applyAutoFullscreenResolution()
{
    auto d = sf::VideoMode::getDesktopMode();
    fullscreenWidth() = d.size.x;
    fullscreenHeight() = d.size.y;
}

void loadConfig(const std::vector<std::string>& mOverridesIds)
{
    ssvu::lo("::loadConfig") << "loading config\n";

    if (ssvufs::Path{"ConfigOverrides/"}.isFolder())
    {
        for (const ssvufs::Path& p :
            ssvufs::getScan<ssvufs::Mode::Single, ssvufs::Type::File,
                ssvufs::Pick::ByExt>("ConfigOverrides/", ".json"))
        {
            if (ssvu::contains(mOverridesIds, p.getFileNameNoExtensions()))
            {
                ssvu::lo("::loadConfig")
                    << "applying config override '"
                    << p.getFileNameNoExtensions() << "'\n";

                const auto overrideRoot(ssvuj::getFromFile(p));
                for (auto itr(std::begin(overrideRoot));
                     itr != std::end(overrideRoot); ++itr)
                {
                    root()[ssvuj::getKey(itr)] = *itr;
                }
            }
        }
    }

    syncAllFromObj();
}

void reapplyResolution()
{
    ssvu::lo("::reapplyResolution") << "reapplying resolution\n";

    if (getWindowedAutoResolution())
    {
        applyAutoWindowedResolution();
    }

    if (getFullscreenAutoResolution())
    {
        applyAutoFullscreenResolution();
    }

    recalculateSizes();
}

void resetConfigToDefaults()
{
    ssvu::lo("::resetConfigToDefaults") << "resetting configs\n";

    resetAllFromDefault();
    reapplyResolution();
}

void resetBindsToDefaults()
{
    ssvu::lo("::resetBindsToDefaults") << "resetting binds to defaults\n";

    resetBindsFromDefault();
}

void saveConfig()
{
    ssvu::lo("::saveConfig") << "saving config\n";
    syncAllToObj();
    ssvuj::writeToFile(root(), "config.json");
}

bool isEligibleForScore()
{
    if (!getOfficial())
    {
        uneligibilityReason = "official mode off";
        return false;
    }

    if (getSpawnDistance() != defaultSpawnDistance)
    {
        uneligibilityReason = "spawn distance modified";
        return false;
    }

    if (getDebug())
    {
        uneligibilityReason = "debug mode on";
        return false;
    }

    if (!getAutoZoomFactor())
    {
        uneligibilityReason = "modified zoom factor";
        return false;
    }

    if (getPlayerSpeed() != playerSpeed().getDefault())
    {
        uneligibilityReason = "player speed modified";
        return false;
    }

    if (getPlayerFocusSpeed() != playerFocusSpeed().getDefault())
    {
        uneligibilityReason = "player focus speed modified";
        return false;
    }

    if (getPlayerSize() != playerSize().getDefault())
    {
        uneligibilityReason = "player size modified";
        return false;
    }

    if (getInvincible())
    {
        uneligibilityReason = "invincibility on";
        return false;
    }

    if (getNoPulse())
    {
        uneligibilityReason = "pulse off";
        return false;
    }

    if (getNoRotation())
    {
        uneligibilityReason = "rotation off";
        return false;
    }

    return true;
}

void recalculateSizes()
{
    sizeX = sizeY = std::max(getWidth(), getHeight()) * 1.3f;

    if (!getAutoZoomFactor())
    {
        return;
    }

    const float factorX(1024.f / ssvu::toFloat(getWidth()));
    const float factorY(768.f / ssvu::toFloat(getHeight()));
    zoomFactor() = std::max(factorX, factorY);
}

void setFullscreen(ssvs::GameWindow& mWindow, bool mFullscreen)
{
    fullscreen() = mFullscreen;

    const sf::Vector2u res{getWidth(), getHeight()};
    mWindow.getRenderWindow().setSize(res);
    mWindow.setSize(res.x, res.y);
    mWindow.setFullscreen(getFullscreen());

    recalculateSizes();
}

void setCurrentResolution(unsigned int mWidth, unsigned int mHeight)
{
    if (getFullscreen())
    {
        fullscreenAutoResolution() = false;
        fullscreenWidth() = mWidth;
        fullscreenHeight() = mHeight;
    }
    else
    {
        windowedAutoResolution() = false;
        windowedWidth() = mWidth;
        windowedHeight() = mHeight;
    }

    recalculateSizes();
}

void setCurrentResolutionAuto(ssvs::GameWindow& mWindow)
{
    if (fullscreen())
    {
        fullscreenAutoResolution() = true;
        applyAutoFullscreenResolution();
    }
    else
    {
        windowedAutoResolution() = true;
        applyAutoWindowedResolution();
    }

    setFullscreen(mWindow, getFullscreen());
}

void setVsync(ssvs::GameWindow& mWindow, bool mValue)
{
    vsync() = mValue;
    mWindow.setVsync(vsync());
}

void setLimitFPS(ssvs::GameWindow& mWindow, bool mValue)
{
    limitFPS() = mValue;
    mWindow.setFPSLimited(mValue);
}

void setMaxFPS(ssvs::GameWindow& mWindow, unsigned int mValue)
{
    maxFPS() = mValue;
    mWindow.setMaxFPS(mValue);
}

void setAntialiasingLevel(ssvs::GameWindow& mWindow, unsigned int mValue)
{
    if (mValue != antialiasingLevel())
    {
        antialiasingLevel() = mValue;
        mWindow.setAntialiasingLevel(mValue);
    }
}

void setOfficial(bool mOfficial)
{
    official() = mOfficial;
}

void setDebug(bool mDebug)
{
    debug() = mDebug;
}

void setNoPulse(bool mNoPulse)
{
    noPulse() = mNoPulse;
}

void setNoRotation(bool mNoRotation)
{
    noRotation() = mNoRotation;
}

void setNoBackground(bool mNoBackground)
{
    noBackground() = mNoBackground;
}

void setBlackAndWhite(bool mBlackAndWhite)
{
    blackAndWhite() = mBlackAndWhite;
}

void setNoSound(bool mNoSound)
{
    noSound() = mNoSound;
}

void setNoMusic(bool mNoMusic)
{
    noMusic() = mNoMusic;
}

void set3D(bool m3D)
{
    _3DEnabled() = m3D;
}

void setShaders(bool mX)
{
    shadersEnabled() = mX;
}

void setInvincible(bool mInvincible)
{
    invincible() = mInvincible;
}

void setAutoRestart(bool mAutoRestart)
{
    autoRestart() = mAutoRestart;
}

void setSoundVolume(float mVolume)
{
    soundVolume() = mVolume;
}

void setMusicVolume(float mVolume)
{
    musicVolume() = mVolume;
}

void setFlash(bool mFlash)
{
    flashEnabled() = mFlash;
}

void setMusicSpeedDMSync(bool mValue)
{
    musicSpeedDMSync() = mValue;
}

void setShowFPS(bool mValue)
{
    showFPS() = mValue;
}

void setMusicSpeedMult(float mValue)
{
    musicSpeedMult() = mValue;
}

void setDrawTextOutlines(bool mX)
{
    drawTextOutlines() = mX;
}

void setDarkenUnevenBackgroundChunk(bool mX)
{
    darkenUnevenBackgroundChunk() = mX;
}

void setRotateToStart(bool mX)
{
    rotateToStart() = mX;
}

void setJoystickDeadzone(float mX)
{
    joystickDeadzone() = mX;
}

void setTextPadding(float mX)
{
    textPadding() = mX;
}

void setTextScaling(float mX)
{
    textScaling() = mX;
}

void setTimescale(float mX)
{
    timescale() = mX;
}

void setShowKeyIcons(bool mX)
{
    showKeyIcons() = mX;
}

void setKeyIconsScale(float mX)
{
    keyIconsScale() = mX;
}

void setFirstTimePlaying(bool mX)
{
    firstTimePlaying() = mX;
}

void setShowLevelInfo(bool mX)
{
    showLevelInfo() = mX;
}

void setShowTimer(bool mX)
{
    showTimer() = mX;
}

void setShowStatusText(bool mX)
{
    showStatusText() = mX;
}

void setServerIp(const std::string& mX)
{
    serverIp() = mX;
}

void setServerPort(unsigned short mX)
{
    serverPort() = mX;
}

void setServerControlPort(unsigned short mX)
{
    serverControlPort() = mX;
}

void setServerLevelWhitelist(const std::vector<std::string>& levelValidators)
{
    serverLevelWhitelist() = levelValidators;
}

void setSaveLastLoginUsername(bool mX)
{
    saveLastLoginUsername() = mX;
}

void setLastLoginUsername(const std::string& mX)
{
    lastLoginUsername() = mX;
}

void setShowLoginAtStartup(bool mX)
{
    showLoginAtStartup() = mX;
}

void setCameraShakeMultiplier(float x)
{
    cameraShakeMultiplier() = x;
}

void setAngleTiltIntensity(float x)
{
    angleTiltIntensity() = x;
}

void setShowPlayerTrail(bool x)
{
    showPlayerTrail() = x;
}

void setPlayerTrailAlpha(unsigned int x)
{
    playerTrailAlpha() = x;
}

void setPlayerTrailScale(float x)
{
    playerTrailScale() = x;
}

void setPlayerTrailDecay(float x)
{
    playerTrailDecay() = x;
}

void setPlayerTrailHasSwapColor(bool x)
{
    playerTrailHasSwapColor() = x;
}

void setShowSwapParticles(bool x)
{
    showSwapParticles() = x;
}

void setPlaySwapReadySound(bool x)
{
    playSwapReadySound() = x;
}

void setShowSwapBlinkingEffect(bool x)
{
    showSwapBlinkingEffect() = x;
}

void setUseLuaFileCache(bool x)
{
    useLuaFileCache() = x;
}

void setDisableGameRendering(bool x)
{
    disableGameRendering() = x;
}

[[nodiscard]] bool getOfficial()
{
    return official();
}

[[nodiscard]] const std::string& getUneligibilityReason()
{
    return uneligibilityReason;
}

[[nodiscard]] float getSizeX()
{
    return sizeX;
}

[[nodiscard]] float getSizeY()
{
    return sizeY;
}

[[nodiscard]] float getSpawnDistance()
{
    return defaultSpawnDistance;
}

[[nodiscard]] float getZoomFactor()
{
    return zoomFactor();
}

[[nodiscard]] int getPixelMultiplier()
{
    return pixelMultiplier();
}

[[nodiscard]] float getPlayerSpeed()
{
    return getOfficial() ? playerSpeed().getDefault() : playerSpeed();
}

[[nodiscard]] float getPlayerFocusSpeed()
{
    return getOfficial() ? playerFocusSpeed().getDefault() : playerFocusSpeed();
}

[[nodiscard]] float getPlayerSize()
{
    return getOfficial() ? playerSize().getDefault() : playerSize();
}

[[nodiscard]] bool getNoPulse()
{
    return getOfficial() ? noPulse().getDefault() : noPulse();
}

[[nodiscard]] bool getNoRotation()
{
    return getOfficial() ? noRotation().getDefault() : noRotation();
}

[[nodiscard]] bool getNoBackground()
{
    return getOfficial() ? noBackground().getDefault() : noBackground();
}

[[nodiscard]] bool getBlackAndWhite()
{
    return getOfficial() ? blackAndWhite().getDefault() : blackAndWhite();
}

[[nodiscard]] bool getNoSound()
{
    return noSound();
}

[[nodiscard]] bool getNoMusic()
{
    return noMusic();
}

[[nodiscard]] float getSoundVolume()
{
    return soundVolume();
}

[[nodiscard]] float getMusicVolume()
{
    return musicVolume();
}

[[nodiscard]] bool getLimitFPS()
{
    return limitFPS();
}

[[nodiscard]] bool getVsync()
{
    return vsync();
}

[[nodiscard]] bool getAutoZoomFactor()
{
    return getOfficial() ? autoZoomFactor().getDefault() : autoZoomFactor();
}

[[nodiscard]] bool getFullscreen()
{
    return fullscreen();
}

[[nodiscard]] bool getWindowedAutoResolution()
{
    return windowedAutoResolution();
}

[[nodiscard]] bool getFullscreenAutoResolution()
{
    return fullscreenAutoResolution();
}

[[nodiscard]] unsigned int getFullscreenWidth()
{
    return fullscreenWidth();
}

[[nodiscard]] unsigned int getFullscreenHeight()
{
    return fullscreenHeight();
}

[[nodiscard]] unsigned int getWindowedWidth()
{
    return windowedWidth();
}

[[nodiscard]] unsigned int getWindowedHeight()
{
    return windowedHeight();
}

[[nodiscard]] unsigned int getWidth()
{
    return getFullscreen() ? getFullscreenWidth() : getWindowedWidth();
}

[[nodiscard]] unsigned int getHeight()
{
    return getFullscreen() ? getFullscreenHeight() : getWindowedHeight();
}

[[nodiscard]] bool getShowMessages()
{
    return showMessages();
}

[[nodiscard]] bool getDebug()
{
    return getOfficial() ? debug().getDefault() : debug();
}

[[nodiscard]] bool getBeatPulse()
{
    return getOfficial() ? beatPulse().getDefault() : beatPulse();
}

[[nodiscard]] bool getInvincible()
{
    return getOfficial() ? invincible().getDefault() : invincible();
}

[[nodiscard]] bool get3D()
{
    return _3DEnabled();
}

[[nodiscard]] bool getShaders()
{
    return shadersEnabled();
}

[[nodiscard]] float get3DMultiplier()
{
    return _3DMultiplier();
}

[[nodiscard]] unsigned int get3DMaxDepth()
{
    return _3DMaxDepth();
}

[[nodiscard]] bool getAutoRestart()
{
    return autoRestart();
}

[[nodiscard]] bool getFlash()
{
    return flashEnabled();
}

[[nodiscard]] bool getShowTrackedVariables()
{
    return showTrackedVariables();
}

[[nodiscard]] bool getMusicSpeedDMSync()
{
    return musicSpeedDMSync();
}

[[nodiscard]] unsigned int getMaxFPS()
{
    return maxFPS();
}

[[nodiscard]] unsigned int getAntialiasingLevel()
{
    return antialiasingLevel();
}

[[nodiscard]] bool getShowFPS()
{
    return showFPS();
}

[[nodiscard]] float getMusicSpeedMult()
{
    return musicSpeedMult();
}

[[nodiscard]] bool getDrawTextOutlines()
{
    return drawTextOutlines();
}

[[nodiscard]] bool getDarkenUnevenBackgroundChunk()
{
    return darkenUnevenBackgroundChunk();
}

[[nodiscard]] bool getRotateToStart()
{
    return rotateToStart();
}

[[nodiscard]] float getJoystickDeadzone()
{
    return joystickDeadzone();
}

[[nodiscard]] float getTextPadding()
{
    return textPadding();
}

[[nodiscard]] float getTextScaling()
{
    return textScaling();
}

[[nodiscard]] float getTimescale()
{
    return getOfficial() ? timescale().getDefault() : timescale();
}

[[nodiscard]] bool getShowKeyIcons()
{
    return showKeyIcons();
}

[[nodiscard]] float getKeyIconsScale()
{
    return keyIconsScale();
}

[[nodiscard]] bool getFirstTimePlaying()
{
    return firstTimePlaying();
}

[[nodiscard]] bool getShowLevelInfo()
{
    return showLevelInfo();
}

[[nodiscard]] bool getShowTimer()
{
    return getOfficial() ? showTimer().getDefault() : showTimer();
}

[[nodiscard]] bool getShowStatusText()
{
    return getOfficial() ? showStatusText().getDefault() : showStatusText();
}

[[nodiscard]] const std::string& getServerIp()
{
    return serverIp();
}

[[nodiscard]] unsigned short getServerPort()
{
    return serverPort();
}

[[nodiscard]] unsigned short getServerControlPort()
{
    return serverControlPort();
}

[[nodiscard]] const std::vector<std::string>& getServerLevelWhitelist()
{
    return serverLevelWhitelist();
}

[[nodiscard]] bool getSaveLastLoginUsername()
{
    return saveLastLoginUsername();
}

[[nodiscard]] const std::string& getLastLoginUsername()
{
    return lastLoginUsername();
}

[[nodiscard]] bool getShowLoginAtStartup()
{
    return showLoginAtStartup();
}

[[nodiscard]] float getCameraShakeMultiplier()
{
    return cameraShakeMultiplier();
}

[[nodiscard]] float getAngleTiltIntensity()
{
    return angleTiltIntensity();
}

[[nodiscard]] bool getShowPlayerTrail()
{
    return showPlayerTrail();
}

[[nodiscard]] unsigned int getPlayerTrailAlpha()
{
    return playerTrailAlpha();
}

[[nodiscard]] float getPlayerTrailScale()
{
    return playerTrailScale();
}

[[nodiscard]] float getPlayerTrailDecay()
{
    return playerTrailDecay();
}

[[nodiscard]] bool getPlayerTrailHasSwapColor()
{
    return playerTrailHasSwapColor();
}

[[nodiscard]] bool getShowSwapParticles()
{
    return showSwapParticles();
}

[[nodiscard]] bool getPlaySwapReadySound()
{
    return playSwapReadySound();
}

[[nodiscard]] bool getShowSwapBlinkingEffect()
{
    return showSwapBlinkingEffect();
}

[[nodiscard]] bool getUseLuaFileCache()
{
    return useLuaFileCache();
}

[[nodiscard]] bool getDisableGameRendering()
{
    return disableGameRendering();
}

//***********************************************************
//
// KEYBOARD/MOUSE BINDS
//
//***********************************************************

//**************************************************
// Game start check

inline constexpr int maxBinds{4};

void resizeTrigger(ssvs::Input::Trigger& trig) noexcept
{
    std::vector<ssvs::Input::Combo>& combos{trig.getCombos()};

    // Remove empty slots to agglomerate all binds
    // close to each other
    auto it{combos.begin()};
    while (it != combos.end())
    {
        if (it->isUnbound())
        {
            combos.erase(it);
            continue;
        }
        ++it;
    }
    // if the config has more binds than are supported
    while (combos.size() > maxBinds)
    {
        combos.pop_back();
    }
    // if the config has less binds fill the
    // spots with unbound combos
    while (combos.size() < maxBinds)
    {
        combos.emplace_back(ssvs::Input::Combo({sf::Keyboard::Key::Unknown}));
    }
}

void keyboardBindsSanityCheck()
{
    resizeTrigger(triggerRotateCCW());
    resizeTrigger(triggerRotateCW());
    resizeTrigger(triggerFocus());
    resizeTrigger(triggerSelect());
    resizeTrigger(triggerExit());
    resizeTrigger(triggerForceRestart());
    resizeTrigger(triggerRestart());
    resizeTrigger(triggerReplay());
    resizeTrigger(triggerScreenshot());
    resizeTrigger(triggerSwap());
    resizeTrigger(triggerUp());
    resizeTrigger(triggerDown());
}

//**************************************************
// Get trigger names

std::string bindToHumanReadableName(std::string s)
{
    if (s.starts_with('k'))
    {
        return s.substr(1);
    }

    if (s == "bLeft")
    {
        return "LMB";
    }

    if (s == "bRight")
    {
        return "RMB";
    }

    if (s == "bMiddle")
    {
        return "MMB";
    }

    return s;
}


const std::array<TriggerGetter, toSizeT(Tid::TriggersCount)> triggerGetters{
    []() -> ssvs::Input::Trigger& { return triggerRotateCCW(); },
    []() -> ssvs::Input::Trigger& { return triggerRotateCW(); },
    []() -> ssvs::Input::Trigger& { return triggerFocus(); },
    []() -> ssvs::Input::Trigger& { return triggerSelect(); },
    []() -> ssvs::Input::Trigger& { return triggerExit(); },
    []() -> ssvs::Input::Trigger& { return triggerForceRestart(); },
    []() -> ssvs::Input::Trigger& { return triggerRestart(); },
    []() -> ssvs::Input::Trigger& { return triggerReplay(); },
    []() -> ssvs::Input::Trigger& { return triggerScreenshot(); },
    []() -> ssvs::Input::Trigger& { return triggerSwap(); },
    []() -> ssvs::Input::Trigger& { return triggerUp(); },
    []() -> ssvs::Input::Trigger& { return triggerDown(); },
    []() -> ssvs::Input::Trigger& { return triggerNextPack(); },
    []() -> ssvs::Input::Trigger& { return triggerPreviousPack(); },
    []() -> ssvs::Input::Trigger& { return triggerLuaConsole(); },
    []() -> ssvs::Input::Trigger& { return triggerPause(); }};

[[nodiscard]] std::string getKeyboardBindNames(const Tid bindID)
{
    int j;
    std::string bindNames;

    const auto combos = triggerGetters.at(toSizeT(bindID))().getCombos();

    for (const auto& c : combos)
    {
        if (c.isUnbound())
        {
            break;
        }

        const auto keyBind{c.getKeys()};
        for (j = 0; j <= static_cast<int>(sf::Keyboard::KeyCount); ++j)
        {
            if (!keyBind[j])
            {
                continue;
            }

            if (!bindNames.empty())
            {
                bindNames += ", ";
            }

            // names are shifted compared to the Key enum
            bindNames += bindToHumanReadableName(
                ssvs::getKKeyName(sf::Keyboard::Key(j - 1)));
            break;
        }

        const auto btnBinds{c.getBtns()};
        for (j = 0; j <= static_cast<int>(sf::Mouse::ButtonCount); ++j)
        {
            if (!btnBinds[j])
            {
                continue;
            }

            if (!bindNames.empty())
            {
                bindNames += ", ";
            }

            // same as with keys
            bindNames += bindToHumanReadableName(
                ssvs::getMBtnName(sf::Mouse::Button(j - 1)));
            break;
        }
    }

    Utils::uppercasify(bindNames);
    return bindNames;
}

//**************************************************
// Add new key binds

void rebindTrigger(ssvs::Input::Trigger& trig, const sf::Keyboard::Key key,
    const sf::Mouse::Button btn, int index)
{
    // if both slots are taken replace the first one
    if (index >= maxBinds)
    {
        index = 0;
        trig.getCombos().at(index).clearBind();
    }

    if (static_cast<int>(key) > -1)
    {
        trig.getCombos().at(index).addKey(key);
    }
    else
    {
        trig.getCombos().at(index).addBtn(btn);
    }
}

//**************************************************
// Unbind key

void clearTriggerBind(ssvs::Input::Trigger& trig, const int index)
{
    trig.getCombos().at(index).clearBind();
}

//***********************************************************
//
// JOYSTICK BINDS
//
//***********************************************************

//**********************************************
// Get binds names

const std::array<JoystickTriggerGetter,
    toSizeT(Joystick::Jid::JoystickBindsCount)>
    joystickTriggerGetters{//
        []() -> unsigned int { return joystickSelect(); },
        []() -> unsigned int { return joystickExit(); },
        []() -> unsigned int { return joystickFocus(); },
        []() -> unsigned int { return joystickSwap(); },
        []() -> unsigned int { return joystickForceRestart(); },
        []() -> unsigned int { return joystickRestart(); },
        []() -> unsigned int { return joystickReplay(); },
        []() -> unsigned int { return joystickScreenshot(); },
        []() -> unsigned int { return joystickNextPack(); },
        []() -> unsigned int { return joystickPreviousPack(); },
        []() -> unsigned int { return joystickAddToFavorites(); },
        []() -> unsigned int { return joystickFavoritesMenu(); }};

std::string getJoystickBindName(const Joystick::Jid bindID)
{
    static std::array<std::array<std::string, 2>, 12> buttonsNames{
        {{"A", "SQUARE"}, {"B", "CROSS"}, {"X", "CIRCLE"}, {"Y", "TRIANGLE"},
            {"LB", "L1"}, {"RB", "R1"}, {"BACK", "L2"}, {"START", "R2"},
            {"LEFT STICK", "SELECT"}, {"RIGHT STICK", "START"},
            {"LT", "LEFT STICK"}, {"RT", "RIGHT STICK"}}};

    const unsigned int value{joystickTriggerGetters[toSizeT(bindID)]()};

    if (value == 33)
    {
        return "";
    }

    constexpr unsigned int msVendorId{0x045E};
    constexpr unsigned int sonyVendorId{0x54C};

    const unsigned int vendorId{
        sf::Joystick::isConnected(0)
            ? sf::Joystick::getIdentification(0).vendorId
            : 0};

    using namespace std::string_literals;

    if (vendorId == msVendorId)
    {
        return value >= 12 ? "" : buttonsNames[value][0];
    }

    if (vendorId == sonyVendorId)
    {
        return value >= 12 ? "" : buttonsNames[value][1];
    }

    return ssvu::toStr(value);
}

//**********************************************
// Get bind

void loadAllJoystickBinds()
{
    for (std::size_t i{0u}; i < Config::joystickTriggerGetters.size(); ++i)
    {
        Joystick::setJoystickBind(Config::joystickTriggerGetters[i](), i);
    }
}

//**********************************************
// Set bind

const std::array<JoystickTriggerSetter,
    toSizeT(Joystick::Jid::JoystickBindsCount)>
    joystickTriggerSetters{//
        [](const unsigned int btn) { joystickSelect() = btn; },
        [](const unsigned int btn) { joystickExit() = btn; },
        [](const unsigned int btn) { joystickFocus() = btn; },
        [](const unsigned int btn) { joystickSwap() = btn; },
        [](const unsigned int btn) { joystickForceRestart() = btn; },
        [](const unsigned int btn) { joystickRestart() = btn; },
        [](const unsigned int btn) { joystickReplay() = btn; },
        [](const unsigned int btn) { joystickScreenshot() = btn; },
        [](const unsigned int btn) { joystickNextPack() = btn; },
        [](const unsigned int btn) { joystickPreviousPack() = btn; },
        [](const unsigned int btn) { joystickAddToFavorites() = btn; },
        [](const unsigned int btn) { joystickFavoritesMenu() = btn; }};

[[nodiscard]] ssvs::Input::Trigger& getTrigger(const Tid tid)
{
    return triggerGetters[toSizeT(tid)]();
}

} // namespace hg::Config
