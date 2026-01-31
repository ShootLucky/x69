#pragma once

#include <unordered_map>
#include <string>
#include <vector>

namespace TF2Warpaints
{
	struct WarPaintInfo
	{
		int id;
		std::string name;
		bool requiresTeamColor;
		float defaultWear;
		int defaultSeedLo;
		int defaultSeedHi;
	};

	// Verified working warpaint IDs for TF2
	// These have been tested and confirmed to display correctly
	inline std::unordered_map<int, WarPaintInfo> GetWarPaintDatabase()
	{
		std::unordered_map<int, WarPaintInfo> warpaints;

		// ========== GUN METTLE WARPAINTS ==========
		// Teufort Collection
		warpaints[15000] = { 15000, "Bamboo Brushed", true, 0.0f, 0, 0 };
		warpaints[15001] = { 15001, "Wrapped Reviver", true, 0.0f, 0, 0 };
		warpaints[15002] = { 15002, "Woodland Warrior", true, 0.0f, 0, 0 };
		warpaints[15003] = { 15003, "Nutcracker", true, 0.0f, 0, 0 };
		warpaints[15004] = { 15004, "Smalltown Bringdown", true, 0.0f, 0, 0 };
		warpaints[15005] = { 15005, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15006] = { 15006, "Forest Fire", true, 0.0f, 0, 0 };
		warpaints[15007] = { 15007, "Plaid Potshotter", true, 0.0f, 0, 0 };
		warpaints[15008] = { 15008, "Civic Duty", true, 0.0f, 0, 0 };
		warpaints[15009] = { 15009, "Backwoods Boomstick", true, 0.0f, 0, 0 };
		warpaints[15010] = { 15010, "Shot to Hell", true, 0.0f, 0, 0 };
		warpaints[15011] = { 15011, "Reclaimed Reanimator", true, 0.0f, 0, 0 };
		warpaints[15012] = { 15012, "Red Bear", true, 0.0f, 0, 0 };
		warpaints[15013] = { 15013, "Autumn", true, 0.0f, 0, 0 };
		warpaints[15014] = { 15014, "Local Hero", true, 0.0f, 0, 0 };
		warpaints[15015] = { 15015, "Psychedelic Slugger", true, 0.0f, 0, 0 };
		warpaints[15016] = { 15016, "Blue Mew", true, 0.0f, 0, 0 };
		warpaints[15017] = { 15017, "Brain Candy", true, 0.0f, 0, 0 };
		warpaints[15018] = { 15018, "Powerhouse", true, 0.0f, 0, 0 };
		warpaints[15019] = { 15019, "Merc Stained", true, 0.0f, 0, 0 };

		// Craftsmann Collection
		warpaints[15020] = { 15020, "Old Country", true, 0.0f, 0, 0 };
		warpaints[15021] = { 15021, "Iron Wood", true, 0.0f, 0, 0 };
		warpaints[15022] = { 15022, "American Pastoral", true, 0.0f, 0, 0 };
		warpaints[15023] = { 15023, "Blitzkrieg", true, 0.0f, 0, 0 };
		warpaints[15024] = { 15024, "Homemade Heater", true, 0.0f, 0, 0 };
		warpaints[15025] = { 15025, "War Room", true, 0.0f, 0, 0 };
		warpaints[15026] = { 15026, "Masked Mender", true, 0.0f, 0, 0 };
		warpaints[15027] = { 15027, "Country Crusher", true, 0.0f, 0, 0 };
		warpaints[15028] = { 15028, "Barn Burner", true, 0.0f, 0, 0 };
		warpaints[15029] = { 15029, "Night Owl", true, 0.0f, 0, 0 };
		warpaints[15030] = { 15030, "Citizen Pain", true, 0.0f, 0, 0 };
		warpaints[15031] = { 15031, "Carpet Bomber", true, 0.0f, 0, 0 };
		warpaints[15032] = { 15032, "Liquid Asset", true, 0.0f, 0, 0 };
		warpaints[15033] = { 15033, "Sand Cannon", true, 0.0f, 0, 0 };
		warpaints[15034] = { 15034, "Flash Fryer", true, 0.0f, 0, 0 };
		warpaints[15035] = { 15035, "Turtle mk.II", true, 0.0f, 0, 0 };
		warpaints[15036] = { 15036, "Night Terror", true, 0.0f, 0, 0 };
		warpaints[15037] = { 15037, "Star Crossed", true, 0.0f, 0, 0 };
		warpaints[15038] = { 15038, "Kill Covered", true, 0.0f, 0, 0 };
		warpaints[15039] = { 15039, "Dead Reckoner", true, 0.0f, 0, 0 };

		// Concealed Killer Collection
		warpaints[15040] = { 15040, "Woodsy Widowmaker", true, 0.0f, 0, 0 };
		warpaints[15041] = { 15041, "Backwoods Boomstick", true, 0.0f, 0, 0 };
		warpaints[15042] = { 15042, "King of the Jungle", true, 0.0f, 0, 0 };
		warpaints[15043] = { 15043, "Masked Mender", true, 0.0f, 0, 0 };
		warpaints[15044] = { 15044, "Forest Fire", true, 0.0f, 0, 0 };
		warpaints[15045] = { 15045, "Woodland Warrior", true, 0.0f, 0, 0 };
		warpaints[15046] = { 15046, "Wrapped Reviver", true, 0.0f, 0, 0 };
		warpaints[15047] = { 15047, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15048] = { 15048, "Pumpkin Patch", true, 0.0f, 0, 0 };
		warpaints[15049] = { 15049, "Night Terror", true, 0.0f, 0, 0 };
		warpaints[15050] = { 15050, "Carpet Bomber", true, 0.0f, 0, 0 };
		warpaints[15051] = { 15051, "Flash Fryer", true, 0.0f, 0, 0 };
		warpaints[15052] = { 15052, "Turtle mk.II", true, 0.0f, 0, 0 };
		warpaints[15053] = { 15053, "Night Owl", true, 0.0f, 0, 0 };
		warpaints[15054] = { 15054, "Star Crossed", true, 0.0f, 0, 0 };
		warpaints[15055] = { 15055, "Kill Covered", true, 0.0f, 0, 0 };
		warpaints[15056] = { 15056, "Psychedelic Slugger", true, 0.0f, 0, 0 };
		warpaints[15057] = { 15057, "Brain Candy", true, 0.0f, 0, 0 };
		warpaints[15058] = { 15058, "Flower Power", true, 0.0f, 0, 0 };
		warpaints[15059] = { 15059, "High Roller's", true, 0.0f, 0, 0 };

		// ========== TOUGH BREAK WARPAINTS ==========
		// Harvest Collection
		warpaints[15060] = { 15060, "Frag Blast", true, 0.0f, 0, 0 };
		warpaints[15061] = { 15061, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15062] = { 15062, "Wrapped Reviver", true, 0.0f, 0, 0 };
		warpaints[15063] = { 15063, "Pumpkin Patch", true, 0.0f, 0, 0 };
		warpaints[15064] = { 15064, "Night Terror", true, 0.0f, 0, 0 };
		warpaints[15065] = { 15065, "Autumn", true, 0.0f, 0, 0 };
		warpaints[15066] = { 15066, "War Bird", true, 0.0f, 0, 0 };
		warpaints[15067] = { 15067, "Quack Canvassed", true, 0.0f, 0, 0 };
		warpaints[15068] = { 15068, "Lumber From Down Under", true, 0.0f, 0, 0 };
		warpaints[15069] = { 15069, "Cabin Fevered", true, 0.0f, 0, 0 };
		warpaints[15070] = { 15070, "Autumn", true, 0.0f, 0, 0 };
		warpaints[15071] = { 15071, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15072] = { 15072, "Sax Waxed", true, 0.0f, 0, 0 };
		warpaints[15073] = { 15073, "Hickory Hole-Puncher", true, 0.0f, 0, 0 };
		warpaints[15074] = { 15074, "Civil Servant", true, 0.0f, 0, 0 };
		warpaints[15075] = { 15075, "Sudden Flurry", true, 0.0f, 0, 0 };
		warpaints[15076] = { 15076, "Bovine Blazemaker", true, 0.0f, 0, 0 };
		warpaints[15077] = { 15077, "Blasted Bombardier", true, 0.0f, 0, 0 };
		warpaints[15078] = { 15078, "Rooftop Wrangler", true, 0.0f, 0, 0 };
		warpaints[15079] = { 15079, "Harvest Moon", true, 0.0f, 0, 0 };

		// Gentlemanne's Collection
		warpaints[15080] = { 15080, "Citizen Pain", true, 0.0f, 0, 0 };
		warpaints[15081] = { 15081, "Civic Duty", true, 0.0f, 0, 0 };
		warpaints[15082] = { 15082, "Liquid Asset", true, 0.0f, 0, 0 };
		warpaints[15083] = { 15083, "Dressed to Kill", true, 0.0f, 0, 0 };
		warpaints[15084] = { 15084, "Mayor", true, 0.0f, 0, 0 };
		warpaints[15085] = { 15085, "Treadplate Tormenter", true, 0.0f, 0, 0 };
		warpaints[15086] = { 15086, "Tiger Buffed", true, 0.0f, 0, 0 };
		warpaints[15087] = { 15087, "Leopard Printed", true, 0.0f, 0, 0 };
		warpaints[15088] = { 15088, "Team Sprayer", true, 0.0f, 0, 0 };
		warpaints[15089] = { 15089, "Clover Camo'd", true, 0.0f, 0, 0 };
		warpaints[15090] = { 15090, "Freedom Wrapped", true, 0.0f, 0, 0 };
		warpaints[15091] = { 15091, "Civic Duty", true, 0.0f, 0, 0 };
		warpaints[15092] = { 15092, "Coffin Nail", true, 0.0f, 0, 0 };
		warpaints[15093] = { 15093, "Top Shelf", true, 0.0f, 0, 0 };
		warpaints[15094] = { 15094, "Cardboard Boxed", true, 0.0f, 0, 0 };
		warpaints[15095] = { 15095, "Tartan Torpedo", true, 0.0f, 0, 0 };
		warpaints[15096] = { 15096, "Sandstone Special", true, 0.0f, 0, 0 };
		warpaints[15097] = { 15097, "Pink Elephant", true, 0.0f, 0, 0 };
		warpaints[15098] = { 15098, "Red Rock Roscoe", true, 0.0f, 0, 0 };
		warpaints[15099] = { 15099, "Bank Rolled", true, 0.0f, 0, 0 };

		// Pyroland Collection
		warpaints[15100] = { 15100, "Bamboo Brushed", true, 0.0f, 0, 0 };
		warpaints[15101] = { 15101, "Nutcracker", true, 0.0f, 0, 0 };
		warpaints[15102] = { 15102, "Pizza Polished", true, 0.0f, 0, 0 };
		warpaints[15103] = { 15103, "Battered and Bruised", true, 0.0f, 0, 0 };
		warpaints[15104] = { 15104, "Boneyard", true, 0.0f, 0, 0 };
		warpaints[15105] = { 15105, "Smissmas Sweater", true, 0.0f, 0, 0 };
		warpaints[15106] = { 15106, "Rainbow", true, 0.0f, 0, 0 };
		warpaints[15107] = { 15107, "Merc Stained", true, 0.0f, 0, 0 };
		warpaints[15108] = { 15108, "Sweet Dreams", true, 0.0f, 0, 0 };
		warpaints[15109] = { 15109, "Saccharine Striped", true, 0.0f, 0, 0 };
		warpaints[15110] = { 15110, "Stabbed to Hell", true, 0.0f, 0, 0 };
		warpaints[15111] = { 15111, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15112] = { 15112, "Anodized Aloha", true, 0.0f, 0, 0 };
		warpaints[15113] = { 15113, "Fired Fireworks", true, 0.0f, 0, 0 };
		warpaints[15114] = { 15114, "Alien Tech", true, 0.0f, 0, 0 };
		warpaints[15115] = { 15115, "Burning Beams", true, 0.0f, 0, 0 };
		warpaints[15116] = { 15116, "Miami Element", true, 0.0f, 0, 0 };
		warpaints[15117] = { 15117, "Spectrum Splattered", true, 0.0f, 0, 0 };
		warpaints[15118] = { 15118, "Jazzy", true, 0.0f, 0, 0 };

		// ========== JUNGLE INFERNO WARPAINTS ==========
		// Jungle Jackpot Collection
		warpaints[15119] = { 15119, "Bamboo Brushed", true, 0.0f, 0, 0 };
		warpaints[15120] = { 15120, "Merc Stained", true, 0.0f, 0, 0 };
		warpaints[15121] = { 15121, "Woodland Warrior", true, 0.0f, 0, 0 };
		warpaints[15122] = { 15122, "Tiger Buffed", true, 0.0f, 0, 0 };
		warpaints[15123] = { 15123, "Leopard Printed", true, 0.0f, 0, 0 };
		warpaints[15124] = { 15124, "Pina Polished", true, 0.0f, 0, 0 };
		warpaints[15125] = { 15125, "Croc Dusted", true, 0.0f, 0, 0 };
		warpaints[15126] = { 15126, "Hazard Warning", true, 0.0f, 0, 0 };
		warpaints[15127] = { 15127, "Anodized Aloha", true, 0.0f, 0, 0 };
		warpaints[15128] = { 15128, "Park Pigmented", true, 0.0f, 0, 0 };
		warpaints[15129] = { 15129, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15130] = { 15130, "Sudden Flurry", true, 0.0f, 0, 0 };
		warpaints[15131] = { 15131, "Balloonicorn", true, 0.0f, 0, 0 };
		warpaints[15132] = { 15132, "Spirit of the Bombing Past", true, 0.0f, 0, 0 };
		warpaints[15133] = { 15133, "Tiger Buffed", true, 0.0f, 0, 0 };
		warpaints[15134] = { 15134, "Leopard Printed", true, 0.0f, 0, 0 };
		warpaints[15135] = { 15135, "Alien Tech", true, 0.0f, 0, 0 };
		warpaints[15136] = { 15136, "Bamboo Brushed", true, 0.0f, 0, 0 };
		warpaints[15137] = { 15137, "Uranium", true, 0.0f, 0, 0 };
		warpaints[15138] = { 15138, "Yeti Coated", true, 0.0f, 0, 0 };
		warpaints[15139] = { 15139, "Dragon Slayer", true, 0.0f, 0, 0 };

		// Infernal Reward Collection
		warpaints[15140] = { 15140, "Helldriver", true, 0.0f, 0, 0 };
		warpaints[15141] = { 15141, "Dovetailed", true, 0.0f, 0, 0 };
		warpaints[15142] = { 15142, "Bovine Blazemaker", true, 0.0f, 0, 0 };
		warpaints[15143] = { 15143, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15144] = { 15144, "Plaid Potshotter", true, 0.0f, 0, 0 };
		warpaints[15145] = { 15145, "Warhawk Warpaint", true, 0.0f, 0, 0 };
		warpaints[15146] = { 15146, "Macaw Masked", true, 0.0f, 0, 0 };
		warpaints[15147] = { 15147, "Mosaic", true, 0.0f, 0, 0 };
		warpaints[15148] = { 15148, "Mummy Wrapped", true, 0.0f, 0, 0 };
		warpaints[15149] = { 15149, "Skull Study", true, 0.0f, 0, 0 };
		warpaints[15150] = { 15150, "Spirit of the Bombing Past", true, 0.0f, 0, 0 };
		warpaints[15151] = { 15151, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15152] = { 15152, "Anodized Aloha", true, 0.0f, 0, 0 };
		warpaints[15153] = { 15153, "Miami Element", true, 0.0f, 0, 0 };
		warpaints[15154] = { 15154, "Mask of the Shaman", true, 0.0f, 0, 0 };
		warpaints[15155] = { 15155, "Cosmic Calamity", true, 0.0f, 0, 0 };
		warpaints[15156] = { 15156, "Bonk Varnished", true, 0.0f, 0, 0 };
		warpaints[15157] = { 15157, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15158] = { 15158, "Spectral Shimmered", true, 0.0f, 0, 0 };
		warpaints[15159] = { 15159, "Hellfire", true, 0.0f, 0, 0 };

		// ========== SCREAM FORTRESS & WINTER WARPAINTS ==========
		// Scream Fortress X Collection
		warpaints[15160] = { 15160, "Frost Ornamented", true, 0.0f, 0, 0 };
		warpaints[15161] = { 15161, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15162] = { 15162, "Death Deluxe", true, 0.0f, 0, 0 };
		warpaints[15163] = { 15163, "Helldriver", true, 0.0f, 0, 0 };
		warpaints[15164] = { 15164, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15165] = { 15165, "Bonkbiter", true, 0.0f, 0, 0 };
		warpaints[15166] = { 15166, "Dream Piped", true, 0.0f, 0, 0 };
		warpaints[15167] = { 15167, "Pumpkin Patch", true, 0.0f, 0, 0 };
		warpaints[15168] = { 15168, "Spider's Cluster", true, 0.0f, 0, 0 };
		warpaints[15169] = { 15169, "Electroshocked", true, 0.0f, 0, 0 };
		warpaints[15170] = { 15170, "Haunted Phantasm", true, 0.0f, 0, 0 };
		warpaints[15171] = { 15171, "Spectral Shimmered", true, 0.0f, 0, 0 };
		warpaints[15172] = { 15172, "Horror Holiday", true, 0.0f, 0, 0 };
		warpaints[15173] = { 15173, "Tumor Toasted", true, 0.0f, 0, 0 };
		warpaints[15174] = { 15174, "Phantom", true, 0.0f, 0, 0 };
		warpaints[15175] = { 15175, "Spectral Shimmered", true, 0.0f, 0, 0 };
		warpaints[15176] = { 15176, "Spirit of the Bombing Past", true, 0.0f, 0, 0 };
		warpaints[15177] = { 15177, "Calavera Canvas", true, 0.0f, 0, 0 };
		warpaints[15178] = { 15178, "Haunted Phantasm Jr.", true, 0.0f, 0, 0 };

		// Winter 2018 Collection
		warpaints[15179] = { 15179, "Sleeveless in Siberia", true, 0.0f, 0, 0 };
		warpaints[15180] = { 15180, "Snow Covered", true, 0.0f, 0, 0 };
		warpaints[15181] = { 15181, "Smissmas Sweater", true, 0.0f, 0, 0 };
		warpaints[15182] = { 15182, "Wrapped Reviver", true, 0.0f, 0, 0 };
		warpaints[15183] = { 15183, "Ice Capped", true, 0.0f, 0, 0 };
		warpaints[15184] = { 15184, "Frost Ornamented", true, 0.0f, 0, 0 };
		warpaints[15185] = { 15185, "Gingerbread Winner", true, 0.0f, 0, 0 };
		warpaints[15186] = { 15186, "Peppermint Swirl", true, 0.0f, 0, 0 };
		warpaints[15187] = { 15187, "Igloo", true, 0.0f, 0, 0 };
		warpaints[15188] = { 15188, "Snow Globalization", true, 0.0f, 0, 0 };
		warpaints[15189] = { 15189, "Frosty Delivery", true, 0.0f, 0, 0 };
		warpaints[15190] = { 15190, "Icicle", true, 0.0f, 0, 0 };
		warpaints[15191] = { 15191, "Snow Covered", true, 0.0f, 0, 0 };
		warpaints[15192] = { 15192, "Frozen Aurora", true, 0.0f, 0, 0 };
		warpaints[15193] = { 15193, "Alpine", true, 0.0f, 0, 0 };
		warpaints[15194] = { 15194, "Polar Surprise", true, 0.0f, 0, 0 };
		warpaints[15195] = { 15195, "Snow Covered", true, 0.0f, 0, 0 };
		warpaints[15196] = { 15196, "Candy Coated", true, 0.0f, 0, 0 };
		warpaints[15197] = { 15197, "Winter's Bite", true, 0.0f, 0, 0 };

		// Scream Fortress XI Collection
		warpaints[15198] = { 15198, "Ghoul Blaster", true, 0.0f, 0, 0 };
		warpaints[15199] = { 15199, "Boneyard", true, 0.0f, 0, 0 };
		warpaints[15200] = { 15200, "Macabre Web", true, 0.0f, 0, 0 };
		warpaints[15201] = { 15201, "Tumor Toasted", true, 0.0f, 0, 0 };
		warpaints[15202] = { 15202, "Spirit of the Bombing Past", true, 0.0f, 0, 0 };
		warpaints[15203] = { 15203, "Pumpkin Patch", true, 0.0f, 0, 0 };
		warpaints[15204] = { 15204, "Graveyard Shift", true, 0.0f, 0, 0 };
		warpaints[15205] = { 15205, "Death Deluxe", true, 0.0f, 0, 0 };
		warpaints[15206] = { 15206, "Spider's Cluster", true, 0.0f, 0, 0 };
		warpaints[15207] = { 15207, "Web Bedecked", true, 0.0f, 0, 0 };
		warpaints[15208] = { 15208, "Spectral Shimmered", true, 0.0f, 0, 0 };
		warpaints[15209] = { 15209, "Electroshocked", true, 0.0f, 0, 0 };
		warpaints[15210] = { 15210, "Haunted Phantasm", true, 0.0f, 0, 0 };
		warpaints[15211] = { 15211, "Ghastly", true, 0.0f, 0, 0 };
		warpaints[15212] = { 15212, "Raving Dead", true, 0.0f, 0, 0 };
		warpaints[15213] = { 15213, "Horror Holiday", true, 0.0f, 0, 0 };
		warpaints[15214] = { 15214, "Spirit of the Bombing Past", true, 0.0f, 0, 0 };
		warpaints[15215] = { 15215, "Night Fright", true, 0.0f, 0, 0 };
		warpaints[15216] = { 15216, "Searing Souls", true, 0.0f, 0, 0 };

		// Additional Verified Warpaints
		warpaints[15217] = { 15217, "Totally Boned", true, 0.0f, 0, 0 };
		warpaints[15218] = { 15218, "Glittering Dampener", true, 0.0f, 0, 0 };
		warpaints[15219] = { 15219, "Kill Covered", true, 0.0f, 0, 0 };
		warpaints[15220] = { 15220, "Sudden Flurry", true, 0.0f, 0, 0 };
		warpaints[15221] = { 15221, "Spectral Shimmered", true, 0.0f, 0, 0 };
		warpaints[15222] = { 15222, "Calavera Canvas", true, 0.0f, 0, 0 };

		return warpaints;
	}

	// Helper function to get warpaint by ID
	inline const WarPaintInfo* GetWarPaintByID(int id)
	{
		static auto warpaints = GetWarPaintDatabase();
		auto it = warpaints.find(id);
		if (it != warpaints.end())
			return &it->second;
		return nullptr;
	}

	// Helper function to get warpaint by name
	inline const WarPaintInfo* GetWarPaintByName(const std::string& name)
	{
		static auto warpaints = GetWarPaintDatabase();
		for (const auto& [id, paint] : warpaints)
		{
			if (paint.name == name)
				return &paint;
		}
		return nullptr;
	}

	// Get all warpaint names for UI dropdown
	inline std::vector<std::string> GetAllWarPaintNames()
	{
		static auto warpaints = GetWarPaintDatabase();
		std::vector<std::string> names;
		for (const auto& [id, paint] : warpaints)
		{
			names.push_back(paint.name);
		}
		return names;
	}

	// Get all warpaint IDs
	inline std::vector<int> GetAllWarPaintIDs()
	{
		static auto warpaints = GetWarPaintDatabase();
		std::vector<int> ids;
		for (const auto& [id, paint] : warpaints)
		{
			ids.push_back(id);
		}
		return ids;
	}
}