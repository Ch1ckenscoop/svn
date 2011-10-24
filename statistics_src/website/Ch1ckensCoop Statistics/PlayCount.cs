using System;
using System.Data;
using System.Configuration;
using System.Linq;
using System.Web;
using System.Web.Security;
using System.Web.UI;
using System.Web.UI.HtmlControls;
using System.Web.UI.WebControls;
using System.Web.UI.WebControls.WebParts;
using System.Xml.Linq;

namespace Ch1ckensCoop_Statistics
{
    public class PlayCount
    {
        private int playCountInt;
        private string mapName;

        public PlayCount()
        {
            playCountInt = 0;
            mapName = "";
        }

        public PlayCount(int count, string name)
        {
            playCountInt = count;
            mapName = name;
        }

        public int PlayCountInt
        {
            get { return playCountInt; }
            set { playCountInt = value; }
        }
        public string MapName
        {
            get { return mapName; }
            set { mapName = value; }
        }
    }
}
