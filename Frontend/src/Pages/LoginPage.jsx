import React, { useState, useEffect } from "react";
import { useNavigate } from "react-router-dom";
import { useAuth } from "./useAuth";
import {
  Paper,
  Typography,
  TextField,
  Button,
  Stack,
  Alert,
  Box
} from "@mui/material";

function LoginPage() {
  const [username, setUsername] = useState("");
  const [password, setPassword] = useState("");
  const [errorMsg, setErrorMsg] = useState("");
  const [statusMsg, setStatusMsg] = useState("");

  const navigate = useNavigate();
  const { login } = useAuth();

  useEffect(() => {
    const userId = sessionStorage.getItem("userId");
    if (userId) {
      navigate("/home");
    }
  }, [navigate]);

  const handleLogin = async (e) => {
    e.preventDefault();
    setErrorMsg("");
    setStatusMsg("");

    try {
      const response = await fetch("https://spendyze.duckdns.org/login", {
        method: "POST",
        headers: {
          "Content-Type": "text/plain"
        },
        body: JSON.stringify({
          username,
          password
        })
      });

      const text = await response.text();
      console.log("Login response:", text);

      if (response.ok && text.includes("successful")) {
        setStatusMsg("Login successful!");
        // Save user ID in session storage
        sessionStorage.setItem("userId", username);
        console.log("User ID stored in session storage:", sessionStorage.getItem("userId"));
        // update auth state
        login();
        // redirect to home page
        navigate("/home");
      } else {
        setErrorMsg(text);
      }
    } catch (err) {
      console.error("Login error:", err);
      setErrorMsg("Network or server error.");
    }
  };

  const handleCreateAccount = async (e) => {
    e.preventDefault();
    setErrorMsg("");
    setStatusMsg("");

    try {
      const response = await fetch("https://spendyze.duckdns.org/create_account", {
        method: "POST",
        headers: {
          "Content-Type": "text/plain"
        },
        body: JSON.stringify({
          username,
          password
        })
      });

      const text = await response.text();
      console.log("Create account response:", text);

      if (response.ok && text.includes("successfully")) {
        setStatusMsg("Account created! You can now log in.");
      } else {
        setErrorMsg(text);
      }
    } catch (err) {
      console.error("Create account error:", err);
      setErrorMsg("Network or server error.");
    }
  };

  return (
    <Box
      display="flex"
      justifyContent="center"
      alignItems="center"
      minHeight="100vh"
    >
      <Paper
        elevation={4}
        sx={{ padding: 4, maxWidth: 400, width: "100%" }}
      >
        <Typography variant="h5" mb={2}>
          Login / Create Account
        </Typography>
        {errorMsg && (
          <Alert severity="error" sx={{ mb: 2 }}>
            {errorMsg}
          </Alert>
        )}
        {statusMsg && (
          <Alert severity="success" sx={{ mb: 2 }}>
            {statusMsg}
          </Alert>
        )}
        <Stack spacing={2} component="form">
          <TextField
            label="Username"
            variant="outlined"
            value={username}
            onChange={(e) => setUsername(e.target.value)}
            required
          />
          <TextField
            label="Password"
            variant="outlined"
            type="password"
            value={password}
            onChange={(e) => setPassword(e.target.value)}
            required
          />
          <Stack direction="row" spacing={2} justifyContent="center">
            <Button variant="contained" onClick={handleLogin}>
              Login
            </Button>
            <Button variant="outlined" onClick={handleCreateAccount}>
              Create Account
            </Button>
          </Stack>
        </Stack>
      </Paper>
    </Box>
  );
}

export default LoginPage;
