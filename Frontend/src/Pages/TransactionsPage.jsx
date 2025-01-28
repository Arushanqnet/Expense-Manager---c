import React, { useState } from "react";
import Navbar from "../Components/NavBar";
import Footer from "../Components/Footer";

import {
  Button,
  FormControl,
  InputLabel,
  MenuItem,
  Select,
  TextField,
  Typography,
  Paper,
  Box,
  Stack,
  Alert,
  Container
} from "@mui/material";
import { LocalizationProvider, DatePicker } from "@mui/x-date-pickers";
import { AdapterDayjs } from "@mui/x-date-pickers/AdapterDayjs";

const AddTransaction = () => {
  const [transactionType, setTransactionType] = useState("expense");
  const [amount, setAmount] = useState("");
  const [date, setDate] = useState(null);
  const [category, setCategory] = useState("");
  const [customCategory, setCustomCategory] = useState("");

  const [successMessage, setSuccessMessage] = useState("");
  const [errorMessage, setErrorMessage] = useState("");

  const expenseCategories = [
    "Food",
    "Transport",
    "Shopping",
    "Rent",
    "Utilities",
    "Miscellaneous",
  ];

  const incomeCategories = [
    "Salary",
    "Business",
    "Investment",
    "Gift",
    "Other",
  ];

  const handleSubmit = async (e) => {
    e.preventDefault();

    // Reset messages
    setSuccessMessage("");
    setErrorMessage("");

    // Prepare the body in naive JSON format
    const requestBody = JSON.stringify({
      type: transactionType,
      amount,
      date: date ? date.format("YYYY-MM-DD") : "",
      category: category || customCategory,
    });

    try {
      const response = await fetch("https://spendyze.duckdns.org/home", {
        method: "POST",
        headers: {
          // The naive C server can handle "text/plain" or "application/json"
          "Content-Type": "text/plain",
        },
        body: requestBody,
      });

      if (response.ok) {
        // Success
        setSuccessMessage("Transaction added successfully!");
      } else {
        // Server error
        setErrorMessage("Error adding transaction. Please try again.");
      }
    } catch (error) {
      console.error("Network or server error:", error);
      setErrorMessage("Unable to connect to the server.");
    }
  };

  const categories = transactionType === "expense" ? expenseCategories : incomeCategories;

  return (
    <>
      <Navbar />

      {/* 
        1) Use full-width container so background spans entire screen.
        2) Add responsive padding for a nicer look.
      */}
      <Container
        maxWidth={false}
        disableGutters
        sx={{
          minHeight: "100vh",
          backgroundColor: "#f1f5f9",
          px: { xs: 2, md: 8 },
          py: { xs: 2, md: 4 },
        }}
      >
        <LocalizationProvider dateAdapter={AdapterDayjs}>
          {/*
            1) Make Paper wider on desktop by using:
               width: { xs: '100%', md: '90%' }
            2) Center it with margin: '0 auto'.
          */}
          <Paper
            elevation={4}
            sx={{
              width: { xs: "100%", md: "90%" },
              maxWidth: "1400px", // optional: cap the max width if you like
              margin: "0 auto",
              p: { xs: 2, md: 4 },
              borderRadius: 3,
            }}
          >
            <Typography
              variant="h5"
              align="center"
              gutterBottom
              sx={{ color: "#333", mb: 2 }}
            >
              Add New Transaction
            </Typography>

            {/* Success/Error Messages */}
            {successMessage && (
              <Alert severity="success" sx={{ mb: 2 }}>
                {successMessage}
              </Alert>
            )}
            {errorMessage && (
              <Alert severity="error" sx={{ mb: 2 }}>
                {errorMessage}
              </Alert>
            )}

            {/*
              2-column responsive grid:
              - Single column (1fr) on mobile (xs)
              - Two columns (1fr 1fr) on desktop (md+)
            */}
            <Box
              component="form"
              onSubmit={handleSubmit}
              sx={{
                display: "grid",
                gridTemplateColumns: { xs: "1fr", md: "1fr 1fr" },
                gap: 3,
              }}
            >
              {/* Transaction Type */}
              <FormControl fullWidth>
                <InputLabel id="transaction-type-label">Type</InputLabel>
                <Select
                  labelId="transaction-type-label"
                  id="transaction-type"
                  value={transactionType}
                  label="Type"
                  onChange={(e) => setTransactionType(e.target.value)}
                >
                  <MenuItem value="expense">Expense</MenuItem>
                  <MenuItem value="income">Income</MenuItem>
                </Select>
              </FormControl>

              {/* Amount */}
              <TextField
                label="Amount"
                variant="outlined"
                type="number"
                value={amount}
                onChange={(e) => setAmount(e.target.value)}
                fullWidth
                required
              />

              {/* Date */}
              <DatePicker
                label="Date"
                value={date}
                onChange={(newValue) => setDate(newValue)}
                renderInput={(params) => (
                  <TextField {...params} required fullWidth />
                )}
              />

              {/* Category */}
              <FormControl fullWidth>
                <InputLabel id="category-select-label">Category</InputLabel>
                <Select
                  labelId="category-select-label"
                  id="category"
                  value={category}
                  label="Category"
                  onChange={(e) => setCategory(e.target.value)}
                >
                  {categories.map((cat) => (
                    <MenuItem key={cat} value={cat}>
                      {cat}
                    </MenuItem>
                  ))}
                  <MenuItem value="">
                    <em>Custom</em>
                  </MenuItem>
                </Select>
              </FormControl>

              {/* Custom Category */}
              {category === "" && (
                <TextField
                  label="Custom Category"
                  variant="outlined"
                  value={customCategory}
                  onChange={(e) => setCustomCategory(e.target.value)}
                  fullWidth
                  required
                />
              )}

              {/* Submit Button (spans both columns on desktop) */}
              <Stack
                direction="row"
                justifyContent="center"
                sx={{ gridColumn: { xs: "1", md: "1 / 3" } }}
              >
                <Button
                  type="submit"
                  variant="contained"
                  size="large"
                  sx={{
                    backgroundColor: "#007aff",
                    ":hover": { backgroundColor: "#005ce6" },
                    textTransform: "none",
                  }}
                >
                  Add Transaction
                </Button>
              </Stack>
            </Box>
          </Paper>
        </LocalizationProvider>
      </Container>

      <Footer />
    </>
  );
};

export default AddTransaction;