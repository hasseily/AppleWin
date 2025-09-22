// argparser.cpp (boost::program_options version)

#include "StdAfx.h"
#include "frontends/common2/argparser.h"
#include "frontends/common2/programoptions.h"
#include "frontends/common2/utils.h"
#include "linux/version.h"
#include "Memory.h"

#include <boost/program_options.hpp>
#include <regex>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace
{
	using namespace boost::program_options;

	void parseGeometry(const std::string &s, std::optional<common2::Geometry> &geometry)
	{
		std::smatch m;
		if (std::regex_match(s, m, std::regex("^(\\d+)x(\\d+)(\\+(\\d+)\\+(\\d+))?$")))
		{
			const size_t groups = m.size();
			if (groups == 6)
			{
				geometry = common2::Geometry();
				geometry->width = std::stoi(m.str(1));
				geometry->height = std::stoi(m.str(2));
				if (!m.str(3).empty())
				{
					geometry->x = std::stoi(m.str(4));
					geometry->y = std::stoi(m.str(5));
				}
				return;
			}
		}
		throw std::runtime_error("Invalid sizes: " + s);
	}

	// Helper to print grouped help like the original
	void print_help(const std::vector<std::pair<std::string, boost::program_options::options_description>> &groups)
	{
		std::cerr << "\n";
		for (const auto &g : groups)
		{
			std::cerr << g.first << ":\n";
			std::ostringstream oss;
			oss << g.second; // program_options does formatted output
			std::cerr << oss.str() << "\n";
		}
	}
} // namespace

namespace common2
{

	bool getEmulatorOptions(
							int argc, char *const argv[], OptionsType type, const std::string &edition, EmulatorOptions &options)
	{
		namespace po = boost::program_options;

		const std::string name = "Apple Emulator for " + edition + " (based on AppleWin " + getVersion() + ")";
		const std::string configurationFileDefault = getConfigFile("applewin.conf").string();

		// ---- Groups ----
		po::options_description top(name);
		// Keep short options where they made sense before; Boost allows digit shorts (e.g., ",1")
		top.add_options()
		("help,h", "Print this help message");

		po::options_description cfg("Configuration");
		cfg.add_options()
		("conf,c", po::value<std::string>()->default_value(configurationFileDefault), "Select configuration file")
		("qt-ini,q", po::bool_switch()->default_value(false), "Use Qt ini file (read only)")
		("registry,r", po::value<std::vector<std::string>>()->composing(), "Registry options section.path=value");

		po::options_description emu("Emulator");
		emu.add_options()
		("log,l", po::bool_switch()->default_value(false), "Log to AppleWin.log")
		("paused", po::bool_switch()->default_value(false), "Start paused")
		("fixed-speed", po::bool_switch()->default_value(false), "Fixed (non-adaptive) speed")
		("headless", po::bool_switch()->default_value(false), "Headless: disable video (freewheel)")
		("benchmark,b", po::bool_switch()->default_value(false), "Benchmark emulator")
		("no-squaring", po::bool_switch()->default_value(false), "Gamepad range is (already) a square")
		("nat", po::value<std::vector<std::string>>()->composing(), "SLIRP PortFwd (e.g. 0,tcp,,8080,,http)");

		po::options_description disk("Disk");
		disk.add_options()
		("d1,1", po::value<std::string>(), "Disk in S6D1 drive")
		("d2,2", po::value<std::string>(), "Disk in S6D2 drive")
		("h1", po::value<std::string>(), "Hard Disk in 1st drive")
		("h2", po::value<std::string>(), "Hard Disk in 2nd drive");

		po::options_description snap("Snapshot");
		snap.add_options()
		("state-filename,f", po::value<std::string>(), "Set snapshot filename")
		("load-state,s", po::value<std::string>(), "Load snapshot from file");

		po::options_description mem("Memory");
		mem.add_options()
		("memclear", po::value<int>(), "Memory initialization pattern [0..7]")
		("rom", po::value<std::string>(), "Custom 12k/16k ROM")
		("f8rom", po::value<std::string>(), "Custom 2k ROM");

		po::options_description audio("Audio");
		audio.add_options()
		("no-audio", po::bool_switch()->default_value(false), "Disable audio")
		("audio-buffer", po::value<unsigned long>()->default_value(options.audioBuffer), "Audio buffer (ms)")
		("wav-speaker", po::value<std::string>(), "Speaker wav output filename")
		("wav-mockingboard", po::value<std::string>(), "Mockingboard wav output filename");

		po::options_description sa2("sa2");
		sa2.add_options()
		("sdl-driver", po::value<int>(), "SDL driver")
		("gl-swap", po::value<int>(), "SDL_GL_SwapInterval")
		("timer", po::bool_switch()->default_value(false), "Synchronise with timer")
		("no-imgui", po::bool_switch()->default_value(false), "Plain SDL2 renderer")
		("geometry", po::value<std::string>(), "WxH[+X+Y]")
		("aspect-ratio", po::bool_switch()->default_value(false), "Always preserve correct aspect ratio")
		("game-controller", po::value<int>(), "SDL_GameControllerOpen")
		("game-mapping-file", po::value<std::string>(), "SDL_GameControllerAddMappingsFromFile")
		("audio-device", po::value<std::string>(), "Audio device name");

		po::options_description applen("applen");
		applen.add_options()
		("no-video-update", po::bool_switch()->default_value(false), "Do not execute NTSC code")
		("ev-device-name", po::value<std::string>(), "Gamepad ev-device name");

		// Compose "all" based on type
		po::options_description all("All options");
		all.add(top).add(cfg).add(emu).add(disk).add(snap).add(mem).add(audio);
		if (type == OptionsType::sa2)      all.add(sa2);
		else if (type == OptionsType::applen) all.add(applen);

		// For pretty, grouped help output like the original
		std::vector<std::pair<std::string, po::options_description>> groups{
			{name, top}, {"Configuration", cfg}, {"Emulator", emu}, {"Disk", disk},
			{"Snapshot", snap}, {"Memory", mem}, {"Audio", audio}
		};
		if (type == OptionsType::sa2)       groups.emplace_back("sa2", sa2);
		else if (type == OptionsType::applen) groups.emplace_back("applen", applen);

		try
		{
			po::variables_map vm;

			// Parse; we don't allow unknown/positional args (to mirror old behavior)
			po::parsed_options parsed = po::command_line_parser(argc, const_cast<char**>(argv))
				.options(all)
				.style(po::command_line_style::default_style |
					   po::command_line_style::allow_short |
					   po::command_line_style::short_allow_next)
				.run();

			// If there are any unrecognized, treat as error with a message close to the old code
			{
				// Re-parse allowing unregistered to detect them
				auto parsed_all = po::command_line_parser(argc, const_cast<char**>(argv))
					.options(all)
					.allow_unregistered()
					.run();
				auto unrec = po::collect_unrecognized(parsed_all.options, po::exclude_positional);
				// Remove recognized options themselves (starting with '-' or '--' are fine if known)
				// Here, if any unrecognized tokens remain that don't look like option keys, error out.
				for (const auto &tok : unrec)
				{
					if (!tok.empty() && tok[0] != '-')
					{
						std::cerr << "Uexpected positional argument: '" << tok << "'\n";
						print_help(groups);
						return false;
					}
				}
			}

			po::store(parsed, vm);
			po::notify(vm);

			// help
			if (vm.count("help"))
			{
				print_help(groups);
				return false;
			}

			// ---- Map to EmulatorOptions ----

			// Configuration
			if (vm.count("conf"))            options.configurationFile = vm["conf"].as<std::string>();
			options.useQtIni                  = vm["qt-ini"].as<bool>();
			if (vm.count("registry"))        options.registryOptions = vm["registry"].as<std::vector<std::string>>();

			// Emulator
			options.log                       = vm["log"].as<bool>();
			if (vm["paused"].as<bool>())     options.autoBoot = false;
			options.fixedSpeed                = vm["fixed-speed"].as<bool>();
			options.headless                  = vm["headless"].as<bool>();
			options.benchmark                 = vm["benchmark"].as<bool>();
			if (vm["no-squaring"].as<bool>()) options.paddleSquaring = false;
			if (vm.count("nat"))              options.natPortFwds = vm["nat"].as<std::vector<std::string>>();

			// Disks
			if (vm.count("d1"))               options.disk1 = vm["d1"].as<std::string>();
			if (vm.count("d2"))               options.disk2 = vm["d2"].as<std::string>();
			if (vm.count("h1"))               options.hardDisk1 = vm["h1"].as<std::string>();
			if (vm.count("h2"))               options.hardDisk2 = vm["h2"].as<std::string>();

			// Snapshot: emulate old mutual exclusivity by precedence:
			if (vm.count("state-filename"))
			{
				options.snapshotFilename = vm["state-filename"].as<std::string>();
				options.loadSnapshot = false;
			}
			if (vm.count("load-state"))
			{
				options.snapshotFilename = vm["load-state"].as<std::string>();
				options.loadSnapshot = true;
			}

			// Memory
			if (vm.count("memclear"))
			{
				const int memclear = vm["memclear"].as<int>();
				if (memclear >= 0 && memclear < NUM_MIP)
					options.memclear = memclear;
				else
				{
					std::cerr << "memclear must be in [0.." << (NUM_MIP - 1) << "]\n";
					print_help(groups);
					return false;
				}
			}
			if (vm.count("rom"))              options.customRom = vm["rom"].as<std::string>();
			if (vm.count("f8rom"))            options.customRomF8 = vm["f8rom"].as<std::string>();

			// Audio
			options.noAudio                   = vm["no-audio"].as<bool>();
			options.audioBuffer               = vm["audio-buffer"].as<unsigned long>();
			if (vm.count("wav-speaker"))      options.wavFileSpeaker = vm["wav-speaker"].as<std::string>();
			if (vm.count("wav-mockingboard")) options.wavFileMockingboard = vm["wav-mockingboard"].as<std::string>();

			// sa2-only
			if (type == OptionsType::sa2)
			{
				if (vm.count("sdl-driver"))       options.sdlDriver = vm["sdl-driver"].as<int>();
				if (vm.count("gl-swap"))          options.glSwapInterval = vm["gl-swap"].as<int>();
				if (vm["timer"].as<bool>())       options.syncWithTimer = true;
				if (vm["no-imgui"].as<bool>())    options.imgui = false;
				if (vm.count("geometry"))
				{
					parseGeometry(vm["geometry"].as<std::string>(), options.geometry);
				}
				options.aspectRatio               = vm["aspect-ratio"].as<bool>();
				if (vm.count("game-controller"))  options.gameControllerIndex = vm["game-controller"].as<int>();
				if (vm.count("game-mapping-file"))options.gameControllerMappingFile = vm["game-mapping-file"].as<std::string>();
				if (vm.count("audio-device"))     options.audioDeviceName = vm["audio-device"].as<std::string>();
			}

			// applen-only
			if (type == OptionsType::applen)
			{
				options.noVideoUpdate             = vm["no-video-update"].as<bool>();
				if (vm.count("ev-device-name"))   options.paddleDeviceName = vm["ev-device-name"].as<std::string>();
			}

			return true;
		}
		catch (const std::exception &e)
		{
			std::cerr << e.what() << "\n";
			print_help(groups);
			return false;
		}
	}

} // namespace common2
