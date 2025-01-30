import React from "react";
import { AppBar, Toolbar, Typography, Button, Box } from "@mui/material";
import { Link, useNavigate } from "react-router-dom";
import { useAuth } from "../Pages/useAuth";

const Navbar = () => {
  const { logout } = useAuth();
  const navigate = useNavigate();

  const handleLogout = () => {
    logout();
    navigate("/");
  };

  return (
    <AppBar
      position="static"
      sx={{
        backgroundColor: "#007aff", // "Multi-billion" style color
      }}
    >
      <Toolbar
        sx={{
          display: "flex",
          justifyContent: "space-between",
        }}
      >
        {/* Logo / Brand Name */}
        <Typography variant="h6" component="div" sx={{ fontWeight: "bold" }}>
          Spendyze
        </Typography>

        {/* Navigation Links */}
        <Box>
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
          
          {/* New Ai Consultation link */}
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
      </Toolbar>
    </AppBar>
  );
};

export default Navbar;
