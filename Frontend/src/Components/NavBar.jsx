import React, { useState } from "react";
import {
  AppBar,
  Toolbar,
  Typography,
  Button,
  Box,
  IconButton,
  SwipeableDrawer,
  List,
  ListItem,
  ListItemButton,
  ListItemText
} from "@mui/material";
import MenuIcon from "@mui/icons-material/Menu";
import { Link, useNavigate } from "react-router-dom";
import { useAuth } from "../Pages/useAuth";

const Navbar = () => {
  const { logout } = useAuth();
  const navigate = useNavigate();
  const [mobileOpen, setMobileOpen] = useState(false);

  const handleLogout = () => {
    logout();
    navigate("/");
  };

  const toggleDrawer = (open) => (event) => {
    if (
      event &&
      (event.type === "keydown" &&
        (event.key === "Tab" || event.key === "Shift"))
    ) {
      return;
    }
    setMobileOpen(open);
  };

  const menuItems = [
    { text: "Add Transaction", path: "/home" },
    { text: "Report", path: "/report" },
    { text: "AI Consultation", path: "/ai-chat" }
  ];

  const drawerContent = (
    <Box
      sx={{ width: 250 }}
      role="presentation"
      onClick={toggleDrawer(false)}
      onKeyDown={toggleDrawer(false)}
    >
      <List>
        {menuItems.map((item) => (
          <ListItem key={item.text} disablePadding>
            <ListItemButton component={Link} to={item.path}>
              <ListItemText primary={item.text} />
            </ListItemButton>
          </ListItem>
        ))}
        <ListItem disablePadding>
          <ListItemButton onClick={handleLogout}>
            <ListItemText primary="Logout" />
          </ListItemButton>
        </ListItem>
      </List>
    </Box>
  );

  return (
    <>
      <AppBar
        position="static"
        sx={{
          backgroundColor: "#007aff"
        }}
      >
        <Toolbar sx={{ display: "flex", justifyContent: "space-between" }}>
          <Typography variant="h6" component="div" sx={{ fontWeight: "bold" }}>
            Spendyze
          </Typography>
          <Box sx={{ display: { xs: "none", md: "flex" } }}>
            <Button
              color="inherit"
              component={Link}
              to="/home"
              sx={{ textTransform: "none", mr: 2 }}
            >
              Add Transaction
            </Button>
            <Button
              color="inherit"
              component={Link}
              to="/report"
              sx={{ textTransform: "none", mr: 2 }}
            >
              Report
            </Button>
            <Button
              color="inherit"
              component={Link}
              to="/ai-chat"
              sx={{ textTransform: "none", mr: 2 }}
            >
              AI Consultation
            </Button>
            <Button
              color="inherit"
              onClick={handleLogout}
              sx={{ textTransform: "none" }}
            >
              Logout
            </Button>
          </Box>
          <Box sx={{ display: { xs: "flex", md: "none" } }}>
            <IconButton
              size="large"
              edge="end"
              color="inherit"
              onClick={toggleDrawer(true)}
            >
              <MenuIcon />
            </IconButton>
          </Box>
        </Toolbar>
      </AppBar>
      <SwipeableDrawer
        anchor="left"
        open={mobileOpen}
        onClose={toggleDrawer(false)}
        onOpen={toggleDrawer(true)}
      >
        {drawerContent}
      </SwipeableDrawer>
    </>
  );
};

export default Navbar;
