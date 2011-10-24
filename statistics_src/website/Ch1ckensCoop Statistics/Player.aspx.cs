using System;
using System.IO;
using System.Linq;
using System.Web.UI.WebControls;
using System.Xml.Linq;

namespace Ch1ckensCoop_Statistics
{
    public partial class Player : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            LoadInfo();
        }

        private void LoadInfo()
        {
            if (Request.QueryString.Count > 0)
            {
                Table1.Rows[1].Cells[0].Width = 265;

                #region Steam Profile Data
                Int64 CID = GetCommunityID(Request.QueryString[0]);
                //bool first = false;

                XDocument doc = XDocument.Load(string.Format("http://steamcommunity.com/profiles/{0}/?xml=1", CID.ToString()));
                //TODO: do something if the player hasn't set up his or her profile.
                //TODO: figure out what (if anything) we can still get info on if someone hasn't set up their profile.
                try
                {
                    if (doc.Descendants("profile").Elements("avatarFull").Single().Value == "http://media.steampowered.com/steamcommunity/public/images/avatars/fe/fef49e7fa7e1997310d705b2a6158ff8dc1cdfeb_full.jpg")
                    {
                        SteamPic.ImageUrl = "question_mark.png";
                    }
                    else
                    {
                        SteamPic.ImageUrl = doc.Descendants("profile").Elements("avatarFull").Single().Value;
                    }
                    Page.Header.Title = string.Format("{0}'s Statistics :: Ch1ckensCoop", doc.Descendants("profile").Elements("steamID").Single().Value);
                    Table1.Rows[0].Cells[2].Text = doc.Descendants("profile").Elements("steamID").Single().Value;

                    if (doc.Descendants("profile").Elements("onlineState").Single().Value.Contains("in-game"))
                    {
                        HyperLink link1 = new HyperLink();
                        link1.NavigateUrl = doc.Descendants("profile").Descendants("inGameInfo").Elements("gameLink").Single().Value;
                        link1.Text = doc.Descendants("profile").Descendants("inGameInfo").Elements("gameName").Single().Value;
                        Label PlayingLable = new Label();
                        PlayingLable.Text = "Playing ";
                        Table1.Rows[1].Cells[1].Controls.Add(PlayingLable);
                        Table1.Rows[1].Cells[1].Controls.Add(link1);
                        if (doc.Descendants("profile").Elements("inGameServerIP").Single().Value.Length > 1)
                        {
                            HyperLink link2 = new HyperLink();
                            link2.Text = "Join Game";
                            link2.NavigateUrl = string.Format("steam://connect/{0}", doc.Descendants("profile").Elements("inGameServerIP").Single().Value);
                            Table1.Rows[2].Cells[1].Text = string.Format("<a href='steam://connect/{0}'>Join Game</a>", doc.Descendants("profile").Elements("inGameServerIP").Single().Value);
                        }
                        else
                        {
                            Table1.Rows[2].Cells[1].Text = "Not in joinable game.";
                        }
                    }
                    else if (doc.Descendants("profile").Elements("onlineState").Single().Value.Contains("online"))
                    {
                        Table1.Rows[1].Cells[1].Text = "Online";
                        Table1.Rows[2].Cells[1].Text = "Not in a game.";
                    }
                    else
                    {
                        Table1.Rows[2].Cells[1].Text = "Not in a game.";
                    }
                    //Get playtime for Alien Swarm
                    XDocument asw_time = XDocument.Load(string.Format("http://steamcommunity.com/profiles/{0}/games?xml=1", CID.ToString()));

                    foreach (XElement game in asw_time.Descendants("gamesList").Descendants("games").Descendants("game"))
                    {
                        //Table1.Rows[4].Cells[1].Text = "Made it this farther...";
                        string value = game.Element("appID").Value.ToString();
                        if (value == "630")
                        {
                            Table1.Rows[4].Cells[1].Text = game.Element("hoursLast2Weeks").Value.ToString() + " Hours   ";
                            Table1.Rows[3].Cells[1].Text = game.Element("hoursOnRecord").Value.ToString() + " Hours   ";
                            Table1.Rows[4].Visible = true;
                            Table1.Rows[3].Visible = true;
                        }
                    }
                    //asw_time.Descendants("gamesList").Descendants("games").Descendants("game").Elements("appID");

                }
                catch
                {
                    //Image Qm = new Image();
                    //Qm.ImageUrl = "questionmark.png";
                    //Response.Write(Qm);
                    Response.Write(@"<img src='question_mark.png'; /> This player has not set up their Steam profile.");
                    Response.End();
                }
                #endregion
                #region Recorded Statistics
                using (StreamReader sr = new StreamReader(@"E:\Ch1ckensCoop_Server\swarm\addons\sourcemod\SwarmStats\players.swarmstats"))
                {
                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        if (line.Contains(Request.QueryString[0]))
                        {
                            while (line.Contains('}') == false)
                            {
                                line = sr.ReadLine();

                                string newline;
                                if (line.Contains(@"""kills"""))
                                {
                                    newline = line.Replace(@"""kills""", "");
                                    newline = newline.Remove(0, newline.IndexOf('"') + 1);
                                    Table1.Rows[5].Cells[1].Text = newline.Remove(newline.Length - 1, 1);
                                }
                                else if (line.Contains(@"""points"""))
                                {
                                    newline = line.Replace(@"""points""", "");
                                    newline = newline.Remove(0, newline.IndexOf('"') + 1);
                                    Table1.Rows[6].Cells[1].Text = newline.Remove(newline.Length - 1, 1);
                                }

                            }
                        }
                    }
                }
                #endregion
                #region Wins and Losses
                using (StreamReader sr = new StreamReader(@"E:\Ch1ckensCoop_Server\swarm\addons\sourcemod\SwarmStats\maps.swarmstats"))
                {
                    int level = 0;
                    int skill = 0;
                    string mapname = "";
                    int wins = -1;
                    int linenume = 0;
                    int plays = -1;

                    string line;
                    while ((line = sr.ReadLine()) != null)
                    {
                        linenume++;
                        if (line.Contains('{'))
                        {
                            level++;
                        }
                        if (level == 1)
                        {
                            if (line.Contains("Easy"))
                                skill = 1;
                            else if (line.Contains("Normal"))
                                skill = 2;
                            else if (line.Contains("Hard"))
                                skill = 3;
                            else if (line.Contains("Insane"))
                                skill = 4;
                            else if (line.Contains("Brutal"))
                                skill = 5;
                        }
                        else if (line.Contains('}'))
                        {
                            level--;
                        }
                        if (line.Contains(Request.QueryString[0]) && level == 3)
                        {
                            while ((line = sr.ReadLine()).Contains('}') == false)
                            {
                                linenume++;
                                string[] data;
                                if (line.Contains(@"""plays"""))
                                {
                                    data = line.Split('"');
                                    plays = Convert.ToInt32(data[3]);
                                }
                                else if (line.Contains(@"""wins"""))
                                {
                                    data = line.Split('"');
                                    wins = Convert.ToInt32(data[3]);
                                }
                            }
                            if (wins == -1)
                                wins = 0;
                            //level--;
                        }
                        if (level == 2)
                        {
                            string[] data = line.Split('"');
                            if (data.Length == 3)
                                mapname = data[1];
                        } 

                        if (mapname != null && wins != -1 && plays != -1)
                        {
                            if (skill == 1)
                            {
                                TableCell mapCell = new TableCell();
                                mapCell.Text = FriendlyNames.GetMapName(mapname);
                                TableCell playsCell = new TableCell();
                                playsCell.Text = (plays - wins).ToString();
                                TableCell winsCell = new TableCell();
                                winsCell.Text = wins.ToString();

                                TableRow row1 = new TableRow();
                                row1.Cells.Add(mapCell);
                                row1.Cells.Add(winsCell);
                                row1.Cells.Add(playsCell);
                                row1.BackColor = System.Drawing.ColorTranslator.FromHtml("#D9F7FF");
                                Easy.Rows.Add(row1);
                                Easy.Visible = true;

                                mapname = "";
                                wins = -1;
                                plays = -1;
                            }
                            else if (skill == 2)
                            {
                                TableCell mapCell = new TableCell();
                                mapCell.Text = FriendlyNames.GetMapName(mapname);
                                TableCell playsCell = new TableCell();
                                playsCell.Text = (plays - wins).ToString();
                                TableCell winsCell = new TableCell();
                                winsCell.Text = wins.ToString();

                                TableRow row1 = new TableRow();
                                row1.Cells.Add(mapCell);
                                row1.Cells.Add(winsCell);
                                row1.Cells.Add(playsCell);
                                row1.BackColor = System.Drawing.ColorTranslator.FromHtml("#D9F7FF");
                                Normal.Rows.Add(row1);
                                Normal.Visible = true;

                                mapname = "";
                                wins = -1;
                                plays = -1;
                            }
                            else if (skill == 3)
                            {
                                TableCell mapCell = new TableCell();
                                mapCell.Text = FriendlyNames.GetMapName(mapname);
                                TableCell playsCell = new TableCell();
                                playsCell.Text = (plays - wins).ToString();
                                TableCell winsCell = new TableCell();
                                winsCell.Text = wins.ToString();

                                TableRow row1 = new TableRow();
                                row1.Cells.Add(mapCell);
                                row1.Cells.Add(winsCell);
                                row1.Cells.Add(playsCell);
                                row1.BackColor = System.Drawing.ColorTranslator.FromHtml("#D9F7FF");
                                Hard.Rows.Add(row1);
                                Hard.Visible = true;

                                mapname = "";
                                wins = -1;
                                plays = -1;
                            }
                            else if (skill == 4)
                            {
                                TableCell mapCell = new TableCell();
                                mapCell.Text = FriendlyNames.GetMapName(mapname);
                                TableCell playsCell = new TableCell();
                                playsCell.Text = (plays - wins).ToString();
                                TableCell winsCell = new TableCell();
                                winsCell.Text = wins.ToString();

                                TableRow row1 = new TableRow();
                                row1.Cells.Add(mapCell);
                                row1.Cells.Add(winsCell);
                                row1.Cells.Add(playsCell);
                                row1.BackColor = System.Drawing.ColorTranslator.FromHtml("#D9F7FF");
                                Insane.Rows.Add(row1);
                                Insane.Visible = true;

                                mapname = "";
                                wins = -1;
                                plays = -1;
                            }
                            else if (skill == 5)
                            {
                                TableCell mapCell = new TableCell();
                                mapCell.Text = FriendlyNames.GetMapName(mapname);
                                TableCell playsCell = new TableCell();
                                playsCell.Text = (plays - wins).ToString();
                                TableCell winsCell = new TableCell();
                                winsCell.Text = wins.ToString();

                                TableRow row1 = new TableRow();
                                row1.Cells.Add(mapCell);
                                row1.Cells.Add(winsCell);
                                row1.Cells.Add(playsCell);
                                row1.BackColor = System.Drawing.ColorTranslator.FromHtml("#D9F7FF");
                                Brutal.Rows.Add(row1);
                                Brutal.Visible = true;

                                mapname = "";
                                wins = -1;
                                plays = -1;
                            }
                        }
                    }
                }
                #endregion
            }
            else
            {
                Response.WriteFile("errors/no_steamid.htm");
                Response.End();
            }
            
            
        }

        public string GetKeyName(string line)
        {
            string[] newline = line.Split('"');
            return newline[1];
        }

        Int64 GetCommunityID(string SteamID)
        {
            String[] parts = SteamID.Split(new char[] { ':' });
            if (parts.Length != 3)
                return 0;
            //string Server = SteamID.Remove(0, SteamID.IndexOf(':') + 1);
            //Server = SteamID.Remove(1);
            int iServer = Convert.ToInt32(parts[1]);

            //string AuthID = SteamID.Remove(0, SteamID.IndexOf(':') + 1);
            //AuthID = SteamID.Remove(0, SteamID.IndexOf(':') + 1);
            int iAuthID = Convert.ToInt32(parts[2]);

            Int64 i64FriendID = Convert.ToInt64(iAuthID * 2);
            i64FriendID += 76561197960265728 + iServer;

            return i64FriendID;
        }

    }
}
