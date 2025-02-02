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

  const expenseCategories = ["Food", "Transport", "Shopping", "Rent", "Utilities", "Miscellaneous"];
  const incomeCategories = ["Salary", "Business", "Investment", "Gift", "Other"];

  const handleSubmit = async (e) => {
    e.preventDefault();
    setSuccessMessage("");
    setErrorMessage("");
    const requestBody = JSON.stringify({
      type: transactionType,
      amount,
      date: date ? date.format("YYYY-MM-DD") : "",
      category: category || customCategory
    });
    try {
      const response = await fetch("https://spendyze.duckdns.org/home", {
        method: "POST",
        headers: { "Content-Type": "text/plain" },
        body: requestBody
      });
      if (response.ok) {
        setSuccessMessage("Transaction added successfully!");
      } else {
        setErrorMessage("Error adding transaction. Please try again.");
      }
    } catch {
      setErrorMessage("Unable to connect to the server.");
    }
  };

  const categories = transactionType === "expense" ? expenseCategories : incomeCategories;

  return (
    <>
      <Navbar />
      <Container
        maxWidth={false}
        disableGutters
        sx={{
          width: "100vw",
          minHeight: "100vh",
          backgroundColor: "#f1f5f9",
          px: { xs: 2, md: 8 },
          py: { xs: 2, md: 4 }
        }}
      >
        <LocalizationProvider dateAdapter={AdapterDayjs}>
          <Paper
            elevation={4}
            sx={{
              width: { xs: "90%", md: "90%" },
              maxWidth: "1400px",
              margin: "0 auto",
              p: { xs: 2, md: 4 },
              borderRadius: 3
            }}
          >
            <Typography variant="h5" align="center" gutterBottom sx={{ color: "#333", mb: 2 }}>
              Add New Transaction
            </Typography>
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
            <Box
              component="form"
              onSubmit={handleSubmit}
              sx={{
                display: "grid",
                gridTemplateColumns: { xs: "1fr", md: "1fr 1fr" },
                gap: 3
              }}
            >
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
              <TextField
                label="Amount"
                variant="outlined"
                type="number"
                value={amount}
                onChange={(e) => setAmount(e.target.value)}
                fullWidth
                required
              />
              <DatePicker
                label="Date"
                value={date}
                onChange={(newValue) => setDate(newValue)}
                renderInput={(params) => <TextField {...params} required fullWidth />}
              />
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
                    textTransform: "none"
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
