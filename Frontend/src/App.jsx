import { useState } from 'react'
import reactLogo from './assets/react.svg'
import './App.css'
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import LoginPage from './Pages/LoginPage';
import TransactionsPage from './Pages/TransactionsPage';
import ReportPage from './Pages/ReportPage';
import { AuthProvider } from './Pages/useAuth';

function PrivateRoute({ children }) {
  const userId = sessionStorage.getItem("userId");
  console.log("userId:", userId);
  return userId ? children : <Navigate to="/" />;
}

function App() {

  return (
    <AuthProvider>
      <Router>
        <Routes>
          <Route path="/" element={<LoginPage />} />
          <Route path="/home" element={<PrivateRoute><TransactionsPage /></PrivateRoute>} />
          <Route path="/report" element={<PrivateRoute><ReportPage /></PrivateRoute>} />
        </Routes>
      </Router>
    </AuthProvider>
  )
}

export default App
