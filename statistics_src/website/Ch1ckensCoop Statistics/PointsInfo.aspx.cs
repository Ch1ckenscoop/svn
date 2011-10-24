using System;
using System.Data;

namespace Ch1ckensCoop_Statistics
{
    public partial class PointsInfo : System.Web.UI.Page
    {
        protected void Page_Load(object sender, EventArgs e)
        {
            #region Points Explanation
            DataTable dtbl2 = new DataTable();
            dtbl2.Columns.Add("Event");
            dtbl2.Columns.Add("Score Change");

            dtbl2.Rows.Add("Any alien killed by sentry", "-1");
            dtbl2.Rows.Add("Rangers", "+2");
            dtbl2.Rows.Add("Uber drones", "+2");
            dtbl2.Rows.Add("Parasite Eggs", "+3");
            dtbl2.Rows.Add("Parasites", "+4");
            dtbl2.Rows.Add("Harvesters", "+5");
            dtbl2.Rows.Add("Mortars", "+7");
            dtbl2.Rows.Add("Biomass", "+10");
            dtbl2.Rows.Add("Boomers", "+10");
            dtbl2.Rows.Add("Shieldbugs", "+15");
            dtbl2.Rows.Add("Queens", "+100");
            dtbl2.Rows.Add("Drones and Other Aliens", "+1");

            PointInfo.DataSource = dtbl2;
            PointInfo.DataBind();
            #endregion

            #region Multiplier Explanation
            DataTable dtbl1 = new DataTable();
            dtbl1.Columns.Add("Difficulty");
            dtbl1.Columns.Add("Multiplier");

            dtbl1.Rows.Add("Easy", "x0.5");
            dtbl1.Rows.Add("Normal", "x1");
            dtbl1.Rows.Add("Hard", "x1.25");
            dtbl1.Rows.Add("Insane", "x1.5");
            dtbl1.Rows.Add("Brutal", "x1.75");
            PointInfo2.DataSource = dtbl1;
            PointInfo2.DataBind();
            #endregion
        }
    }
}
