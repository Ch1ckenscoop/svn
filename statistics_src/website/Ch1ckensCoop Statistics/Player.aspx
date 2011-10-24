<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Player.aspx.cs" Inherits="Ch1ckensCoop_Statistics.Player" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>Player Statistics :: Ch1ckensCoop</title>
    <style type="text/css">
        body
        {
	        background: url("asw.png") 0% 100% no-repeat;
	        background-attachment:fixed;
        }
    </style>
</head>
<body style="font-family: Arial, Helvetica, sans-serif">
    <form id="form1" runat="server">
    <asp:Table ID="Table1" runat="server" GridLines="Both" BorderColor="Black" 
        BorderWidth="1px" HorizontalAlign="Center">
        <asp:TableRow runat="server">
            <asp:TableCell runat="server" Height="184px" RowSpan="8" Width="184px" 
                BorderWidth="1px" BorderColor="Black"><asp:Image ID="SteamPic" runat="server" /></asp:TableCell>
            <asp:TableCell Font-Bold="True" ForeColor="White" BackColor="#00B3E4" runat="server" BorderWidth="1px" BorderColor="Black">Steam Name</asp:TableCell>
            <asp:TableCell BackColor="#D9F7FF" runat="server" ID="nickname">steam nickname here</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server">
            <asp:TableCell Font-Bold="True" ForeColor="White" BackColor="#00B3E4" runat="server" BorderWidth="1px" BorderColor="Black">Status</asp:TableCell>
            <asp:TableCell BackColor="#D9F7FF" runat="server">Offline</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server">
            <asp:TableCell Font-Bold="True" ForeColor="White" BackColor="#00B3E4" runat="server" BorderWidth="1px" BorderColor="Black">Game 
            Server</asp:TableCell>
            <asp:TableCell BackColor="#D9F7FF" runat="server">game name here</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" Visible="False">
            <asp:TableCell Font-Bold="True" ForeColor="White" BackColor="#00B3E4" runat="server" BorderWidth="1px" BorderColor="Black">Alien Swarm Time</asp:TableCell>
            <asp:TableCell BackColor="#D9F7FF" runat="server" >alien swarm time here</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" Visible="False">
            <asp:TableCell Font-Bold="True" ForeColor="White" BackColor="#00B3E4" runat="server" BorderWidth="1px" BorderColor="Black">Alien Swarm Time Last 2 weeks</asp:TableCell>
            <asp:TableCell BackColor="#D9F7FF" runat="server">alien swarm time last two weeks here</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server">
            <asp:TableCell Font-Bold="True" ForeColor="White" BackColor="#00B3E4" runat="server" BorderWidth="1px" BorderColor="Black">Kills</asp:TableCell>
            <asp:TableCell BackColor="#D9F7FF" runat="server">kills here</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server">
            <asp:TableCell Font-Bold="True" ForeColor="White" runat="server" BackColor="#00B3E4">Points</asp:TableCell>
            <asp:TableCell runat="server" BackColor="#D9F7FF">points here</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" Visible="False">
            <asp:TableCell Font-Bold="True" ForeColor="White" runat="server" BackColor="#00B3E4">Rank</asp:TableCell>
            <asp:TableCell runat="server" BackColor="#D9F7FF">rank here</asp:TableCell>
        </asp:TableRow>
    </asp:Table>
    <br />
    <asp:Table ID="Easy" runat="server" GridLines="Both" BorderColor="Black" 
        BorderWidth="1px" HorizontalAlign="Center" Visible="False">
        <asp:TableRow runat="server" BackColor="#00B3E4" BorderColor="Black" 
            BorderStyle="Solid" BorderWidth="1px" ForeColor="White">
            <asp:TableCell runat="server" ColumnSpan="3" Font-Bold="True" 
                HorizontalAlign="Center">Map Statistics - Easy</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" BackColor="#00B3E4" ForeColor="White">
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Map</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Wins</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Losses</asp:TableCell>
        </asp:TableRow>
    </asp:Table>
    <br />
    <asp:Table ID="Normal" runat="server" GridLines="Both" BorderColor="Black" 
        BorderWidth="1px" HorizontalAlign="Center" Visible="False">
        <asp:TableRow runat="server" BackColor="#00B3E4" BorderColor="Black" 
            BorderStyle="Solid" BorderWidth="1px" ForeColor="White">
            <asp:TableCell runat="server" ColumnSpan="3" Font-Bold="True" 
                HorizontalAlign="Center">Map Statistics - Normal</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" BackColor="#00B3E4" ForeColor="White">
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Map</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Wins</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Losses</asp:TableCell>
        </asp:TableRow>
    </asp:Table>
    <br />
    <asp:Table ID="Hard" runat="server" GridLines="Both" BorderColor="Black" 
        BorderWidth="1px" HorizontalAlign="Center" Visible="False">
        <asp:TableRow runat="server" BackColor="#00B3E4" BorderColor="Black" 
            BorderStyle="Solid" BorderWidth="1px" ForeColor="White">
            <asp:TableCell runat="server" ColumnSpan="3" Font-Bold="True" 
                HorizontalAlign="Center">Map Statistics - Hard</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" BackColor="#00B3E4" ForeColor="White">
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Map</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Wins</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Losses</asp:TableCell>
        </asp:TableRow>
    </asp:Table>
    <br />
    <asp:Table ID="Insane" runat="server" GridLines="Both" BorderColor="Black" 
        BorderWidth="1px" HorizontalAlign="Center" Visible="False">
        <asp:TableRow runat="server" BackColor="#00B3E4" BorderColor="Black" 
            BorderStyle="Solid" BorderWidth="1px" ForeColor="White">
            <asp:TableCell runat="server" ColumnSpan="3" Font-Bold="True" 
                HorizontalAlign="Center">Map Statistics - Insane</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" BackColor="#00B3E4" ForeColor="White">
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Map</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Wins</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Losses</asp:TableCell>
        </asp:TableRow>
    </asp:Table>
    <br />
    <asp:Table ID="Brutal" runat="server" GridLines="Both" BorderColor="Black" 
        BorderWidth="1px" HorizontalAlign="Center" Visible="False">
        <asp:TableRow runat="server" BackColor="#00B3E4" BorderColor="Black" 
            BorderStyle="Solid" BorderWidth="1px" ForeColor="White">
            <asp:TableCell runat="server" ColumnSpan="3" Font-Bold="True" 
                HorizontalAlign="Center">Map Statistics - Brutal</asp:TableCell>
        </asp:TableRow>
        <asp:TableRow runat="server" BackColor="#00B3E4" ForeColor="White">
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Map</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Wins</asp:TableCell>
            <asp:TableCell runat="server" BorderColor="Black" BorderStyle="Solid" 
                BorderWidth="1px" Font-Bold="True" HorizontalAlign="Center">Losses</asp:TableCell>
        </asp:TableRow>
    </asp:Table>
    </form>
</body>
</html>
