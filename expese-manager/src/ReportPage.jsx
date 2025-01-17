import React, { useState, useEffect } from "react";
// For Chart.js 3 or 4, react-chartjs-2 automatically registers necessary components
import { Bar } from "react-chartjs-2";

function ReportPage() {
  const [data, setData] = useState(null);
  const [error, setError] = useState("");

  useEffect(() => {
    // Fetch your combined JSON endpoint
    fetch("/transactions")
      .then((res) => {
        if (!res.ok) {
          throw new Error(`Server responded with ${res.status}`);
        }
        return res.json();
      })
      .then((json) => {
        // 'json' should have { method1: [...], method2: {...} }
        setData(json);
      })
      .catch((err) => {
        console.error(err);
        setError("Error fetching transactions data");
      });
  }, []);

  if (error) {
    return <div style={{ color: "red" }}>{error}</div>;
  }

  if (!data) {
    // Still loading
    return <div>Loading...</div>;
  }

  // data has { method1, method2 }:
  const { method1, method2 } = data; // method1 = array of tx, method2 = bar chart config

  return (
    <div style={{ padding: 20 }}>
      <h1>Transactions</h1>

      {/* 
        method2 is the entire Chart.js config: 
        {
          type: "bar",
          data: { labels: [...], datasets: [...] },
          options: { scales: {...} }
        }
        BUT with react-chartjs-2, we typically pass "data" and "options" to the Bar component.
        We'll ignore 'type', because <Bar> is already "bar."
      */}
      <h2>Bar Chart</h2>
      <div style={{ width: "600px", marginBottom: "40px" }}>
        <Bar data={method2.data} options={method2.options} />
      </div>

      <h2>All Transactions (Table)</h2>
      <table
        border="1"
        cellPadding="5"
        style={{ borderCollapse: "collapse", width: "100%", maxWidth: "800px" }}
      >
        <thead>
          <tr>
            <th>ID</th>
            <th>Type</th>
            <th>Amount</th>
            <th>Date</th>
            <th>Category</th>
          </tr>
        </thead>
        <tbody>
          {method1.map((tx) => (
            <tr key={tx.id}>
              <td>{tx.id}</td>
              <td>{tx.trans_type}</td>
              <td>{tx.amount}</td>
              <td>{tx.date}</td>
              <td>{tx.category}</td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  );
}

export default ReportPage;
