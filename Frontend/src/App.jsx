import { useState } from 'react'
import reactLogo from './assets/react.svg'
import './App.css'
import { BrowserRouter as Router, Routes, Route, Navigate } from 'react-router-dom';
import LoginPage from './Pages/LoginPage';
import TransactionsPage from './Pages/TransactionsPage';
import ReportPage from './Pages/ReportPage';
import AiChatPage from './Pages/AiChatPage';   // <-- Import your new AI Chat page

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
          {/* Public route */}
          <Route path="/" element={<LoginPage />} />
          
          {/* Private routes */}
          <Route path="/home" element={<PrivateRoute><TransactionsPage /></PrivateRoute>} />
          <Route path="/report" element={<PrivateRoute><ReportPage /></PrivateRoute>} />
          
          {/* Add your new AI Chat route here. It can also be private if you want only logged-in users to access it. */}
          <Route path="/ai-chat" element={<PrivateRoute><AiChatPage /></PrivateRoute>} />
        </Routes>
      </Router>
    </AuthProvider>
  )
}

export default App
