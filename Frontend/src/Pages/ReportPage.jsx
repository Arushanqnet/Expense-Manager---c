import React, { useEffect, useState } from "react";
import Navbar from "../Components/NavBar";
import Footer from "../Components/Footer";
import {
  Typography,
  Container,
  Card,
  CardContent,
  Table,
  TableHead,
  TableRow,
  TableCell,
  TableBody,
  Box
} from "@mui/material";
import {
  Chart as ChartJS,
  CategoryScale,
  LinearScale,
  BarElement,
  Title,
  Tooltip,
  Legend
} from "chart.js";
import { Bar } from "react-chartjs-2";

ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend);

function ReportPage() {
  const [tableData, setTableData] = useState([]);
  const [chartConfig, setChartConfig] = useState(null);

  useEffect(() => {
    fetch("https://spendyze.duckdns.org/transactions")
      .then((res) => res.json())
      .then((data) => {
        setTableData(data.method1);
        setChartConfig(data.method2);
      })
      .catch((err) => console.error("Error fetching transactions:", err));
  }, []);

  return (
    <>
      <Navbar />
      <Container
        maxWidth={false}
        disableGutters
        sx={{
          width: "100vw",
          minHeight: "93vh",
          backgroundColor: "#f1f5f9",
          px: { xs: 2, md: 8 },
          py: { xs: 2, md: 4 }
        }}
      >
        <Box sx={{ display: "flex", flexDirection: "column" }}>
          <Container
            maxWidth="xl"
            sx={{ py: 4, flex: 1, display: "flex", flexDirection: "column" }}
          >
            <Typography variant="h4" gutterBottom sx={{ color: "black" }}>
              My Transactions
            </Typography>
            <Card elevation={3}>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Transaction History
                </Typography>
                <Box sx={{ maxHeight: 400, overflow: "auto" }}>
                  <Table>
                    <TableHead>
                      <TableRow>
                        <TableCell>Type</TableCell>
                        <TableCell>Amount</TableCell>
                        <TableCell>Date</TableCell>
                        <TableCell>Category</TableCell>
                      </TableRow>
                    </TableHead>
                    <TableBody>
                      {tableData.map((tx) => (
                        <TableRow key={tx.id}>
                          <TableCell>{tx.trans_type}</TableCell>
                          <TableCell>{tx.amount}</TableCell>
                          <TableCell>{tx.date}</TableCell>
                          <TableCell>{tx.category}</TableCell>
                        </TableRow>
                      ))}
                    </TableBody>
                  </Table>
                </Box>
              </CardContent>
            </Card>
            <br />
            <Card elevation={3} sx={{ height: "100%" }}>
              <CardContent>
                <Typography variant="h6" gutterBottom>
                  Monthly Report
                </Typography>
                {chartConfig && (
                  <Box sx={{ height: 450 }}>
                    <Bar data={chartConfig.data} options={chartConfig.options} />
                  </Box>
                )}
              </CardContent>
            </Card>
          </Container>
        </Box>
      </Container>
      <Footer />
    </>
  );
}

export default ReportPage;
