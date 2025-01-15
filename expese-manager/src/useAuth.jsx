import React, { createContext, useContext, useState } from "react";

// Create a context to store auth state & actions
const AuthContext = createContext(null);

// AuthProvider that wraps your app
export function AuthProvider({ children }) {
  const [isLoggedIn, setIsLoggedIn] = useState(false);

  const login = () => {
    setIsLoggedIn(true);
  };
  
  const logout = () => {
    setIsLoggedIn(false);
  };

  return (
    <AuthContext.Provider value={{ isLoggedIn, login, logout }}>
      {children}
    </AuthContext.Provider>
  );
}

// Custom hook to quickly access auth context
export function useAuth() {
  return useContext(AuthContext);
}
