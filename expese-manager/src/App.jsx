import React from "react";
import { BrowserRouter, Routes, Route } from "react-router-dom";
import LoginPage from "./LoginPage";
import TransactionsPage from "./TransactionsPage";
import { AuthProvider } from "./useAuth";
import ProtectedRoute from "./ProtectedRoute";

function App() {
  return (
    <AuthProvider>
      <BrowserRouter>
        <Routes>
          {/* Root route = Login page */}
          <Route path="/" element={<LoginPage />} />

          {/* Protected route for transaction page */}
          <Route
            path="/transactions"
            element={
              <ProtectedRoute>
                <TransactionsPage />
              </ProtectedRoute>
            }
          />
        </Routes>
      </BrowserRouter>
    </AuthProvider>
  );
}

export default App;
