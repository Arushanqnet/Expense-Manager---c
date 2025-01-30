import React, { useState, useEffect } from 'react';
import { Container, Box, Typography, Card, CardContent, TextField, Button, Paper } from '@mui/material';
import { styled } from '@mui/material/styles';
import Navbar from "../Components/NavBar";
import Footer from "../Components/Footer";

const GEMINI_API_KEY = "AIzaSyByjsdEWdIf0bJZ-L6svcUNCpTwnFz04nE";

// Styled component for a modern “chat bubble”
const MessageBubble = styled(Paper)(({ theme, sender }) => ({
  padding: theme.spacing(1.5, 2),
  marginBottom: theme.spacing(1.5),
  maxWidth: '70%',
  alignSelf: sender === 'user' ? 'flex-end' : 'flex-start',
  // Colors for user vs AI
  backgroundColor: sender === 'user' ? '#E0FFE1' : '#FFFFFF',
  borderRadius: sender === 'user'
    ? '16px 0px 16px 16px'
    : '0px 16px 16px 16px',
}));

/**
 * Parses any text containing **bold** markers and returns an array of
 * React fragments with <strong> elements where appropriate.
 */
const parseBoldText = (text) => {
  const boldRegex = /\*\*(.*?)\*\*/g;
  const elements = [];
  let lastIndex = 0;
  let match;

  while ((match = boldRegex.exec(text)) !== null) {
    // Push the text before the match (if any)
    if (match.index > lastIndex) {
      elements.push(text.substring(lastIndex, match.index));
    }
    // Push the bold text
    elements.push(<strong key={match.index}>{match[1]}</strong>);
    lastIndex = boldRegex.lastIndex;
  }

  // Push any remaining text after the last match
  if (lastIndex < text.length) {
    elements.push(text.substring(lastIndex));
  }
  
  return elements;
};

const AiChatPage = () => {
  const [tableData, setTableData] = useState([]);
  const [chartConfig, setChartConfig] = useState(null);
  const [messages, setMessages] = useState([]);
  const [userInput, setUserInput] = useState("");

  // Fetch data from server on mount
  useEffect(() => {
    fetch('https://spendyze.duckdns.org/transactions')
      .then((res) => res.json())
      .then((data) => {
        setTableData(data.method1 || []);
        setChartConfig(data.method2 || null);
      })
      .catch((err) => console.error('Error fetching transactions:', err));
  }, []);

  const handleSend = async () => {
    if (!userInput.trim()) return;

    // Add user message to the chat
    setMessages((prev) => [...prev, { sender: 'user', text: userInput }]);

    const requestBody = {
      contents: [
        {
          parts: [
            { 
              text: `User query: ${userInput}\n\nTransactions data:\n${JSON.stringify(tableData)}`
            }
          ]
        }
      ]
    };

    // Clear input field
    setUserInput("");

    try {
      const response = await fetch(
        `https://generativelanguage.googleapis.com/v1beta/models/gemini-pro:generateContent?key=${GEMINI_API_KEY}`,
        {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(requestBody)
        }
      );

      if (!response.ok) {
        throw new Error(`API request failed with status ${response.status}`);
      }

      const data = await response.json();
      // The Gemini Pro response structure includes candidates => content => parts => text
      const generatedText = data?.candidates?.[0]?.content?.parts
        ?.map((part) => part.text)
        .join(" ") || "No response text found.";

      // Add AI response to the chat
      setMessages((prev) => [...prev, { sender: 'ai', text: generatedText }]);
    } catch (error) {
      console.error("Error calling Gemini API:", error);
      setMessages((prev) => [
        ...prev,
        { sender: 'ai', text: "Error or invalid response from the AI." }
      ]);
    }
  };

  return (
    <>
      <Navbar />
      <Container
        maxWidth="sm"
        sx={{
          minHeight: "100vh",
          display: "flex",
          flexDirection: "column",
          backgroundColor: "#f2f3f5",
          py: 2
        }}
      >
        <Typography variant="h4" align="center" gutterBottom sx={{ mt: 2 }}>
          AI Chat
        </Typography>
        
        {/* Chat Area */}
        <Box
          sx={{
            flex: 1,
            display: "flex",
            flexDirection: "column",
            overflowY: "auto",
            mb: 2
          }}
        >
          {messages.map((msg, index) => (
            <Box
              key={index}
              sx={{
                display: "flex",
                flexDirection: "column",
                alignItems: msg.sender === 'user' ? 'flex-end' : 'flex-start'
              }}
            >
              <MessageBubble elevation={1} sender={msg.sender}>
                {parseBoldText(msg.text)}
              </MessageBubble>
            </Box>
          ))}
        </Box>

        {/* Input Area */}
        <Card sx={{ mb: 2 }}>
          <CardContent
            sx={{
              display: "flex",
              alignItems: "center",
              justifyContent: "space-between"
            }}
          >
            <TextField
              variant="outlined"
              fullWidth
              placeholder="Type your message..."
              value={userInput}
              onChange={(e) => setUserInput(e.target.value)}
              onKeyDown={(e) => {
                if (e.key === "Enter") {
                  e.preventDefault();
                  handleSend();
                }
              }}
            />
            <Button 
              variant="contained" 
              color="primary" 
              onClick={handleSend} 
              sx={{ ml: 1 }}
            >
              Send
            </Button>
          </CardContent>
        </Card>
      </Container>
      <Footer />
    </>
  );
};

export default AiChatPage;
