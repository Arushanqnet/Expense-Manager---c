import React from "react";
import { Box, Typography } from "@mui/material";

const Footer = () => {
  return (
    <Box
      component="footer"
      sx={{
        backgroundColor: "#f1f5f9",
        color: "#333",
        textAlign: "center",
        py: 2,
        mt: "auto",
      }}
    >
      <Typography variant="body2" color="text.secondary">
        © {new Date().getFullYear()} Billion Corp. All rights reserved.
      </Typography>
    </Box>
  );
};

export default Footer;