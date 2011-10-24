<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="PointsInfo.aspx.cs" Inherits="Ch1ckensCoop_Statistics.PointsInfo" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>Scoring Information</title>
</head>
<body>
    <form id="form1" runat="server">
    <div>
    
    <asp:GridView ID="PointInfo" runat="server" CellPadding="2" ForeColor="#333333" 
        GridLines="None" style="font-family: Arial, Helvetica, sans-serif">
        <RowStyle BackColor="#D9F7FF" />
        <FooterStyle BackColor="#507CD1" Font-Bold="True" ForeColor="White" />
        <PagerStyle BackColor="#2461BF" ForeColor="White" HorizontalAlign="Center" />
        <SelectedRowStyle BackColor="#D1DDF1" Font-Bold="True" ForeColor="#333333" />
        <HeaderStyle BackColor="#00B3E4" Font-Bold="True" ForeColor="White" />
        <EditRowStyle BackColor="#2461BF" />
        <AlternatingRowStyle BackColor="White" />
    </asp:GridView>
    
        <br />
    
    <asp:GridView ID="PointInfo2" runat="server" CellPadding="2" ForeColor="#333333" 
        GridLines="None" style="font-family: Arial, Helvetica, sans-serif">
        <RowStyle BackColor="#D9F7FF" />
        <FooterStyle BackColor="#507CD1" Font-Bold="True" ForeColor="White" />
        <PagerStyle BackColor="#2461BF" ForeColor="White" HorizontalAlign="Center" />
        <SelectedRowStyle BackColor="#D1DDF1" Font-Bold="True" ForeColor="#333333" />
        <HeaderStyle BackColor="#00B3E4" Font-Bold="True" ForeColor="White" />
        <EditRowStyle BackColor="#2461BF" />
        <AlternatingRowStyle BackColor="White" />
    </asp:GridView>
    
    </div>
    </form>
</body>
</html>
