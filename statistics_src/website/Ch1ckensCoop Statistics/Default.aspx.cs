using System;
using System.Collections;
using System.Data;
using System.IO;
using RCON;
using System.Net;
using System.Web;
using System.Web.UI;
using System.Web.UI.WebControls;
using GoogleChartSharp;

namespace Ch1ckensCoop_Statistics
{
    public partial class _Default : System.Web.UI.Page
    {
        internal DataTable dtbl1;
        internal SourceRcon rcon;
        protected void Page_Load(object sender, EventArgs e)
        {
            #region Map Statistics

            ArrayList playCountArray = new ArrayList();

            using (StreamReader sr = new StreamReader(@"E:\Ch1ckensCoop_Server\swarm\addons\sourcemod\SwarmStats\maps.swarmstats"))
            {
                int level = 0;
                int skill = 0;
                string mapname = "";
                int playcountint = 0;
                string line;
                while ((line = sr.ReadLine()) != null)
                {
                    if (line.Contains("{"))
                    {
                        level++;
                        if (level == 1)
                        {
                            line = sr.ReadLine();
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
                    }
                    else if (line.Contains("}"))
                    {
                        level--;
                        if (playcountint != 0 && mapname != "" && level == 2)
                        {
                            PlayCount pc1 = new PlayCount();
                            pc1.PlayCountInt = playcountint;
                            pc1.MapName = FriendlyNames.GetCampaignName(mapname);
                            if (pc1.MapName != "cancel_now")
                                playCountArray.Add(pc1);
                            playcountint = 0;
                        }
                    }
                    if (level == 2)
                    {
                        string[] data = line.Split('"');
                        if (data.Length > 1)
                            mapname = data[1];
                    }
                    if (level == 4 && line.Contains("plays"))
                    {
                        string[] data = line.Split('"');
                        playcountint += Convert.ToInt32(data[3]);
                    }

                }

                playCountArray.Add(new PlayCount(1, "z_null_map"));
                IComparer sortByMapNameComparer = new sortByMapNameClass();
                playCountArray.Sort(sortByMapNameComparer);

                string previousmapname = "";
                int playcount = 0;
                bool firstTime = true;
                ArrayList dupe = new ArrayList(playCountArray);
                playCountArray.Clear();
                foreach (PlayCount pc in dupe)
                {
                    if (firstTime)
                    {
                        previousmapname = pc.MapName;
                        firstTime = false;
                    }
                    if (previousmapname != pc.MapName)
                    {
                        PlayCount pc2 = new PlayCount();
                        pc2.PlayCountInt = playcount;
                        pc2.MapName = previousmapname;
                        if (pc2.PlayCountInt != 0 && pc2.MapName != "" && pc2.MapName != "Jacob's Rest")
                            playCountArray.Add(pc2);
                        playcount = 0;
                        previousmapname = pc.MapName;
                    }
                    playcount += pc.PlayCountInt;
                }
                ArrayList labels = new ArrayList();
                ArrayList values = new ArrayList();
                foreach (PlayCount pc in playCountArray)
                {
                    labels.Add(pc.MapName);
                    values.Add(pc.PlayCountInt);
                }

                float total = 0;
                foreach (int value in values)
                {
                    total += value;
                }
                total -= 1;
                ArrayList valuesDupe = new ArrayList(values);
                values.Clear();
                foreach (int value in valuesDupe)
                {
                    values.Add(value / total);
                }

                string[] labelsArray = (string[])labels.ToArray(typeof(string));
                float[] valuesArray = (float[])values.ToArray(typeof(float));

                PieChart chart = new PieChart(600, 300);
                chart.SetData(valuesArray);
                chart.SetPieChartLabels(labelsArray);
                chart.SetTitle("Custom Map Statistics");
                chart.SetDatasetColors(new string[] { "00B3E4" });
                Image12.ImageUrl = chart.GetUrl() + "&chf=bg,s,65432100";
            }
            #endregion
            #region Player Statistics
            string name = "";
            int kills = 0;
            int points = 0;

            dtbl1 = new DataTable();
            dtbl1.Columns.Add("  ");
            dtbl1.Columns.Add("Total Kills", Type.GetType("System.Int32"));
            dtbl1.Columns.Add("Points", Type.GetType("System.Int32"));
            dtbl1.Columns.Add(" ");
            //TODO: Highlight admins in red.
            try
            {
                using (StreamReader sr = new StreamReader(@"E:\Ch1ckensCoop_Server\swarm\addons\sourcemod\SwarmStats\players.swarmstats"))
                {
                    int level = 0;
                    string steamid = "";
                    String line;
                    // Read and display lines from the file until the end of
                    // the file is reached.
                    while ((line = sr.ReadLine()) != null)
                    {
                        string[] steamid_temp;
                        if (!line.Contains("Players"))
                        {
                            if (line.Contains("{"))
                            {
                                level++;
                            }
                            else if (line.Contains("}"))
                            {
                                level--;
                                if (name.Length > 0)
                                {
                                    //HyperLink link1 = new HyperLink();
                                    //link1.Text = name;
                                    string url = string.Format("Player.aspx?={0}", steamid);
                                    dtbl1.Rows.Add(name, kills, points, url);
                                }
                                name = "";
                                kills = 0;
                                points = 0;
                            }
                            else if (line.Contains("STEAM_"))
                            {
                                //TableRow row1 = new TableRow();
                                steamid_temp = line.Split('"');
                                steamid = steamid_temp[1];
                                //Response.Write("'" + steamid_temp[1] + "'");
                            }
                            else
                            {
                                string newline;

                                if (line.Contains(@"""name"""))
                                {
                                    newline = line.Replace(@"""name""", "");
                                    newline = newline.Remove(0, newline.IndexOf('"') + 1);
                                    newline = newline.Remove(newline.Length - 1, 1);
                                    name = newline;
                                }
                                else if (line.Contains(@"""kills"""))
                                {
                                    newline = line.Replace(@"""kills""", "");
                                    newline = newline.Remove(0, newline.IndexOf('"') + 1);
                                    newline = newline.Remove(newline.Length - 1, 1);
                                    kills = Convert.ToInt32(float.Parse(newline));
                                }
                                else if (line.Contains(@"""points"""))
                                {
                                    newline = line.Replace(@"""points""", "");
                                    newline = newline.Remove(0, newline.IndexOf('"') + 1);
                                    newline = newline.Remove(newline.Length - 1, 1);
                                    points = Convert.ToInt32(float.Parse(newline));
                                }
                            }
                        }
                    }
                }
                Session["dtbl"] = dtbl1;
                dtbl1.DefaultView.Sort = "Points DESC";
                GridView1.DataSource = dtbl1;
                GridView1.DataBind();
                //GridView1.Sort("Points", SortDirection.Descending);

                /*DataTable ranktable = new DataTable();
                ranktable.Columns.Add("Rank", Type.GetType("System.Int32"));
                foreach (DataRow row in dtbl1.Rows)
                {
                    ranktable.Rows.Add(rank++);
                }
                GridView2.DataSource = ranktable;
                GridView2.DataBind();*/
            }
            catch
            {

            }
            #endregion

            /*#region Current Players
            rcon = new SourceRcon();
            IPEndPoint point = new IPEndPoint(IPAddress.Parse("192.168.1.120"), 27015);
            rcon.ConnectionSuccess += new BoolInfo(rcon_ConnectionSuccess);
            rcon.Connect(point, "pwnappleftwlol");
            #endregion*/
        }
        /*
        void rcon_ConnectionSuccess(bool info)
        {
            try
            {
                if (info)
                {
                    rcon.ServerOutput += new StringOutput(rcon_ServerOutput);
                    rcon.ServerCommand("status");
                }
            }
            catch
            {
            }
        }

        void rcon_ServerOutput(string output)
        {
            UpdatePanelText.Text = output;
            UpdatePanel1.Update();
        }
        */
        protected void GridView1_Sorting(object sender, GridViewSortEventArgs e)
        {
            DataTable dt = Session["dtbl"] as DataTable;

            if (dt != null)
            {

                //Sort the data.
                dt.DefaultView.Sort = e.SortExpression + " " + GetSortDirection(e.SortExpression);
                GridView1.DataSource = Session["dtbl"];
                GridView1.DataBind();
            }
        }

        private string GetSortDirection(string column)
        {
            if (column != "Player Name")
            {
                // By default, set the sort direction to Descending.
                string sortDirection = "DESC";

                // Retrieve the last column that was sorted.
                string sortExpression = ViewState["SortExpression"] as string;

                if (sortExpression != null)
                {
                    // Check if the same column is being sorted.
                    // Otherwise, the default value can be returned.
                    if (sortExpression == column)
                    {
                        string lastDirection = ViewState["SortDirection"] as string;
                        if ((lastDirection != null) && (lastDirection == "DESC"))
                        {
                            sortDirection = "ASC";
                        }
                    }
                }

                // Save new values in ViewState.
                ViewState["SortDirection"] = sortDirection;
                ViewState["SortExpression"] = column;

                return sortDirection;
            }
            else
            {
                // By default, set the sort direction to Descending.
                string sortDirection = "ASC";

                // Retrieve the last column that was sorted.
                string sortExpression = ViewState["SortExpression"] as string;

                if (sortExpression != null)
                {
                    // Check if the same column is being sorted.
                    // Otherwise, the default value can be returned.
                    if (sortExpression == column)
                    {
                        string lastDirection = ViewState["SortDirection"] as string;
                        if ((lastDirection != null) && (lastDirection == "ASC"))
                        {
                            sortDirection = "DESC";
                        }
                    }
                }

                // Save new values in ViewState.
                ViewState["SortDirection"] = sortDirection;
                ViewState["SortExpression"] = column;

                return sortDirection;
            }
        }

        protected void Button1_Click(object sender, EventArgs e)
        {

        }

        protected void Button1_Click1(object sender, EventArgs e)
        {
            Response.Redirect("http://ibemad.dyndns.org:8080/ch1ckenscoop/stats");
        }

        protected void GridView1_DataBound(object sender, EventArgs e)
        {
            
            foreach (GridViewRow row in GridView1.Rows)
            {
                row.Cells[1].Text = "<a href='" + row.Cells[5].Text + "'>" + row.Cells[2].Text + "</a>";
                row.Cells[5].Text = "";
                row.Cells[2].Text = "";
            }
            
        }
    }
    public class sortByMapNameClass : IComparer
    {
        int IComparer.Compare(Object x, Object y)
        {
            return ((PlayCount)x).MapName.CompareTo(((PlayCount)y).MapName);
        }
    }
}
