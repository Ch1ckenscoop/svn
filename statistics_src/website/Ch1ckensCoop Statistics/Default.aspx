<%@ Page Language="C#" AutoEventWireup="true" CodeBehind="Default.aspx.cs" Inherits="Ch1ckensCoop_Statistics._Default" %>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">

<html xmlns="http://www.w3.org/1999/xhtml" >
<head runat="server">
    <title>Leaderboard :: Ch1ckensCoop</title>
    <style type="text/css">
        #form1
        {
            font-family: Arial, Helvetica, sans-serif;
        }
        body
        {
	        background: url("asw.png") 0% 100% no-repeat;
	        background-attachment:fixed;
        }
    </style>
</head>
<body>
    <form id="form1" runat="server">
    <asp:HyperLink ID="newtestWindow" Runat="server" 
        NavigateUrl="javascript:w= window.open('PointsInfo.aspx','mywin','left=20,top=20,width=325,height=325,toolbar=0,resizable=0');" 
        Width="100%" >Scoring Information</asp:HyperLink>
    <br />
    <br />
    <asp:Button ID="Button1" runat="server"
        Text="Refresh Stats" onclick="Button1_Click1" style="text-align: left" />
    <asp:ScriptManager ID="ScriptManager1" runat="server" EnableHistory="True" 
        EnablePageMethods="True" EnableScriptGlobalization="True">
    </asp:ScriptManager>
    <br />
    <br />
    These statistics are reset every time a new map is released. For detailed 
    statistics, click on a player&#39;s name.<br />
    <br />
    <div style="text-align: center;">
        <asp:Image ID="Image12" runat="server" 
            align="middle" Height="300px" Width="600px"/></div>
    <asp:GridView ID="GridView1" runat="server" AllowSorting="True" CellPadding="4" 
        Font-Names="Arial" ForeColor="#333333" GridLines="None" 
        onsorting="GridView1_Sorting" HorizontalAlign="Center" BorderColor="Black" 
        BorderWidth="1px" ondatabound="GridView1_DataBound">
        <RowStyle BackColor="#D9F7FF" />
        <FooterStyle BackColor="#507CD1" Font-Bold="True" ForeColor="White" />
        <PagerStyle BackColor="#2461BF" ForeColor="White" HorizontalAlign="Center" />
        <SelectedRowStyle BackColor="#D1DDF1" Font-Bold="True" ForeColor="#333333" />
        <HeaderStyle BackColor="#00B3E4" Font-Bold="True" ForeColor="White" BorderWidth="1px" BorderColor="Black" BorderStyle="Solid" />
        <EditRowStyle BackColor="#2461BF" />
        <AlternatingRowStyle BackColor="White" />
        <Columns>
        <asp:TemplateField HeaderText="Rank">
  <ItemTemplate>
    <%# Container.DataItemIndex + 1 %>
  </ItemTemplate>
</asp:TemplateField>
            <asp:BoundField HeaderText="Player Name" HtmlEncode="False">
                <ControlStyle Width="0px" />
                <FooterStyle Width="0px" />
                <HeaderStyle Width="0px" />
                <ItemStyle Width="0px" />
            </asp:BoundField>
        </Columns>
    </asp:GridView>
</div>
    </form>
</body>
</html>
