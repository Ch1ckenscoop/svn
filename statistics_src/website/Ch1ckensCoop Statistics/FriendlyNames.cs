
namespace Ch1ckensCoop_Statistics
{
    public class FriendlyNames
    {
        public static string GetMapName(string rawMapName)
        {
            if (rawMapName == "as_city17_01")
                return "Trainstation";
            else if (rawMapName == "as_city17_02")
                return "City Center";
            else if (rawMapName == "as_city17_03")
                return "Tunnel";
            else if (rawMapName == "as_city17_04")
                return "Fountain";
            else if (rawMapName == "as_city17_05")
                return "Citadel";
            else if (rawMapName == "dc1-omega_city")
                return "Omega City";
            else if (rawMapName == "dc2-breaking_an_entry")
                return "Breaking An Entry";
            else if (rawMapName == "dc3-search_and_rescue")
                return "Search And Rescue";
            else if (rawMapName == "as_paranoia1")
                return "Unexpected Encounter";
            else if (rawMapName == "as_paranoia2")
                return "Hostile Places";
            else if (rawMapName == "as_paranoia3")
                return "High Tension";
            else if (rawMapName == "edge1colony")
                return "Colony";
            else if (rawMapName == "edge2crash")
                return "Crash";
            else if (rawMapName == "edge3truckin")
                return "Truckin'";
            else if (rawMapName == "edge4relay")
                return "Relay";
            else if (rawMapName == "edge5theship")
                return "The Ship";
            else if (rawMapName == "as_sci1_bridge")
                return "Lana's Bridge";
            else if (rawMapName == "as_sci2_sewer")
                return "Lana's Sewer";
            else if (rawMapName == "as_sci3_maintenance")
                return "Lana's Maintenance";
            else if (rawMapName == "as_sci4_vent")
                return "Lana's Vents";
            else if (rawMapName == "as_sci5_complex")
                return "Lana's Complex";
            else if (rawMapName == "westcliff1")
                return "Westcliff Refinery Outpost";
            else if (rawMapName == "asi_muffin_survival")
                return "Juno Outpost Survival";
            else if (rawMapName == "TFT-DesertOutpost" || rawMapName == "tft-desertoutpost")
                return "Insertion Point";
            else if (rawMapName == "TFT-AbandonedMaintenance")
                return "Abandoned Maintenance Tunnels";
            else if (rawMapName == "TFT-Spaceport")
                return "Oasis Colony Spaceport";
            else if (rawMapName == "SmallRoom")
                return "Small Room";
            else if (rawMapName == "LargeRoom")
                return "Large Room";
            else if (rawMapName == "ocs_1")
                return "Storage Facility";
            else if (rawMapName == "ocs_2")
                return "Landing Bay 7";
            else if (rawMapName == "ocs_3")
                return "U.S.C. Medusa";
            else if (rawMapName == "syntek_hospital")
                return "Full Treatment";
            else if (rawMapName == "royalh1")
                return "Royal Harvest";
            else if (rawMapName == "ASI-Jac1-LandingBay_01")
                return "Landing Bay";
            else if (rawMapName == "ASI-Jac1-LandingBay_02")
                return "Cargo Elevator";
            else if (rawMapName == "ASI-Jac2-Deima")
                return "Deima Surface Bridge";
            else if (rawMapName == "ASI-Jac3-Rydberg")
                return "Rydberg Reactor";
            else if (rawMapName == "ASI-Jac4-Residential")
                return "Syntek Residential";
            else if (rawMapName == "ASI-Jac6-SewerJunction")
                return "Sewer Junction";
            else if (rawMapName == "ASI-Jac7-TimorStation")
                return "Timor Station";
            else if (rawMapName == "asi-lostcause")
                return "Lost Cause";
            else if (rawMapName == "arctic-infiltration")
                return "Arctic Infiltration";
            else if (rawMapName == "asi-sj1-coldcatwalksbeta1" || rawMapName == "as_coldcatwalks")
                return "Cold Catwalks";
            else if (rawMapName == "asi-surv1-deepfreeze")
                return "Deep Freeze Survival";
            else if (rawMapName == "cube")
                return "Cube";
            else if (rawMapName == "xen-cathalu-labs")
                return "Call To Cathalu";
            else
                return rawMapName;
        }


        public static string GetCampaignName(string rawMapName)
        {
            if (rawMapName == "as_city17_01" || rawMapName == "as_city17_02" || rawMapName == "as_city17_03" || rawMapName == "as_city17_04" || rawMapName == "as_city17_05")
                return "City 17";
            else if (rawMapName == "dc1-omega_city" || rawMapName == "dc2-breaking_an_entry" || rawMapName == "dc3-search_and_rescue")
                return "Dead City";
            else if (rawMapName == "as_paranoia1" || rawMapName == "as_paranoia2" || rawMapName == "as_paranoia3")
                return "Paranoia";
            else if (rawMapName == "edge1colony" || rawMapName == "edge2crash" || rawMapName == "edge3truckin" || rawMapName == "edge4relay" || rawMapName == "edge5theship")
                return "The Edge";
            else if (rawMapName == "as_sci1_bridge" || rawMapName == "as_sci2_sewer" || rawMapName == "as_sci3_maintenance" || rawMapName == "as_sci4_vent" || rawMapName == "as_sci5_complex")
                return "Lana's Escape";
            else if (rawMapName == "westcliff1")
                return "Westcliff Refinery Outpost";
            else if (rawMapName == "asi_muffin_survival")
                return "Juno Outpost";
            else if (rawMapName == "TFT-DesertOutpost" || rawMapName == "tft-desertoutpost" || rawMapName == "TFT-Spaceport" || rawMapName == "TFT-AbandonedMaintenance")
                return "Tears for Tarnor";
            else if (rawMapName == "smallroom" || rawMapName == "LargeRoom" || rawMapName == "SmallRoom")
                return "Holdout";
            else if (rawMapName == "ocs_1" || rawMapName == "ocs_2" || rawMapName == "ocs_3")
                return "Operation Cleansweep";
            else if (rawMapName == "syntek_hospital")
                return "Full Treatment";
            else if (rawMapName == "royalh1")
                return "Royal Harvest";
            else if (rawMapName == "ASI-Jac1-LandingBay_01" || rawMapName == "ASI-Jac1-LandingBay_02" || rawMapName == "ASI-Jac2-Deima" || rawMapName == "ASI-Jac3-Rydberg" || rawMapName == "ASI-Jac4-Residential" || rawMapName == "ASI-Jac6-SewerJunction" || rawMapName == "ASI-Jac7-TimorStation")
                return "Jacob's Rest";
            else if (rawMapName == "asi-lostcause")
                return "Lost Cause";
            else if (rawMapName == "arctic-infiltration")
                return "Arctic Infiltration";
            else if (rawMapName == "asi-sj1-coldcatwalksbeta1" || rawMapName == "as_coldcatwalks")
                return "Cold Catwalks";
            else if (rawMapName == "asi-surv1-deepfreeze")
                return "Deep Freeze";
            else if (rawMapName == "cube")
                return "Cube";
            else if (rawMapName == "lobby")
                return "cancel_now";
            else if (rawMapName == "xen-cathalu-labs")
                return "Call To Cathalu";
            else
                return rawMapName;
        }
    }
}
