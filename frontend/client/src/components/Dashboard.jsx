import axios from "axios";
import React, { useState, useEffect, useContext, useRef } from "react";
import { UserContext } from "../context/UserContext";
import { useNavigate } from "react-router-dom";

// TODO https://github.com/t20e/trading-app/blob/main/client/src/components/Dashboard.jsx
const Dashboard = () => {
    const { loggedUser, setLoggedUser } = useContext(UserContext);
    const redirect = useNavigate();

    return <div></div>;
};

export default Dashboard;