import React, { useState } from "react";
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
  Alert
} from "@mui/material";
import { LocalizationProvider, DatePicker } from "@mui/x-date-pickers";
import { AdapterDayjs } from "@mui/x-date-pickers/AdapterDayjs";

const AddTransaction = () => {
  const [transactionType, setTransactionType] = useState("expense");
  const [amount, setAmount] = useState("");
  const [date, setDate] = useState(null);
  const [category, setCategory] = useState("");

  const [successMessage, setSuccessMessage] = useState("");
  const [errorMessage, setErrorMessage] = useState("");

  // Example categories
  const categories = [
    "Food",
    "Transport",
    "Shopping",
    "Rent",
    "Utilities",
    "Miscellaneous",
  ];

  const handleSubmit = async (e) => {
    e.preventDefault();

    // Reset messages
    setSuccessMessage("");
    setErrorMessage("");

    // Prepare the body in a naive JSON format (the C backend's naive parser
    // looks for "type", "amount", "date", "category" between quotes)
    const requestBody = JSON.stringify({
      type: transactionType,
      amount,
      date: date ? date.format("YYYY-MM-DD") : "",
      category,
    });

    try {
      const response = await fetch("http://localhost:8080/home", {
        method: "POST",
        headers: {
          // The C server uses a naive parser, so "Content-Type: text/plain" can work,
          // but "application/json" is also acceptable if your server handles it.
          "Content-Type": "text/plain",
        },
        body: requestBody,
      });

      if (response.ok) {
        // For demonstration, the server returns "Data inserted OK." on success
        setSuccessMessage("Transaction added successfully!");
      } else {
        // The server might return 500 or something else on error
        setErrorMessage("Error adding transaction. Please try again.");
      }
    } catch (error) {
      console.error("Network or server error:", error);
      setErrorMessage("Unable to connect to the server.");
    }

    // Optionally reset form fields
    // setTransactionType("expense");
    // setAmount("");
    // setDate(null);
    // setCategory("");
  };

  return (
    <LocalizationProvider dateAdapter={AdapterDayjs}>
      <Paper
        elevation={4}
        sx={{
          maxWidth: 500,
          margin: "40px auto",
          padding: "30px",
          borderRadius: 3,
          backgroundColor: "#f1f5f9",
        }}
      >
        <Typography variant="h5" align="center" gutterBottom color="#333">
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

        <Box
          component="form"
          onSubmit={handleSubmit}
          sx={{ display: "flex", flexDirection: "column", gap: 2 }}
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
            renderInput={(params) => <TextField {...params} required />}
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
              required
            >
              {categories.map((cat) => (
                <MenuItem key={cat} value={cat}>
                  {cat}
                </MenuItem>
              ))}
            </Select>
          </FormControl>

          {/* Submit Button */}
          <Stack direction="row" justifyContent="center">
            <Button
              type="submit"
              variant="contained"
              size="large"
              sx={{
                backgroundColor: "#007aff",
                ":hover": { backgroundColor: "#005ce6" },
              }}
            >
              Add Transaction
            </Button>
          </Stack>
        </Box>
      </Paper>
    </LocalizationProvider>
  );
};

export default AddTransaction;
