import axios from "axios";
import React, { useState, useEffect, useContext, useRef } from "react";
import { UserContext } from "../context/UserContext";
import { useNavigate } from "react-router-dom";

// TODO https://github.com/t20e/trading-app/blob/main/client/src/components/Dashboard.jsx
const Dashboard = () => {
    const { loggedUser, setLoggedUser, isLoading } = useContext(UserContext);
    const redirect = useNavigate();
    const [usersList, setUsersList] = useState([]);
    const [renderImage, setRenderImage] = useState(null);

    const getImage = () => {
        axios
            .get("http://localhost:8080/api/getRandomImage", {
                withCredentials: true,
                responseType: "blob", // Expects binary data
            })
            .then((res) => {
                console.log("all users:", res.data);
                if (renderImage) {
                    // If there was a previous image, clean up memory
                    URL.revokeObjectURL(renderImage);
                }
                // Create temp URL that points to the binary image data in the browser's memory
                const imageUrl = URL.createObjectURL(res.data);
                setRenderImage(imageUrl);
            })
            .catch((err) => {
                console.log(err);
                alert("Failed to get image!");
            });
    };

    // Clean up memory when component is destroyed
    useEffect(() => {
        return () => {
            if (renderImage) {
                URL.revokeObjectURL(renderImage);
            }
        };
    }, [renderImage]);

    useEffect(() => {
        // Do run logic if it's still checking the session token.
        if (isLoading) return;

        // Make sure the user is logged in else redirect to login
        if (loggedUser === null) {
            console.log("User not logged in redirecting...");
            redirect("/");
            return;
        }

        if (usersList.length === 0) {
            console.log("Getting all users.");
            axios
                .get("http://localhost:8080/api/getAllusers", {
                    withCredentials: true, // The browser will send the JWT token cookie automatically, if it has it!
                })
                .then((res) => {
                    console.log("all users:", res.data.users);
                    if (res.data.users) {
                        setUsersList(
                            res.data.users.filter(
                                // Filter out the current logged user
                                (user) => user.userID !== loggedUser.userID
                            )
                        );
                    }
                })
                .catch((err) => {
                    console.log(err);
                    alert("Could not get all other users.");
                });
        }
    }, [isLoading, loggedUser, redirect]);

    if (isLoading) {
        return (
            <div className="text-center py-5">
                <p>Loading session...</p>
            </div>
        );
    }

    const handleLogout = () => {
        console.log("Logging user out...");
        // Because we are using an HTTP-only session cookie (JWT-token) for user authentication, and javascript isn't allowed to access it. We need to request the backend to send the frontend browser info so we can delete the session cookie, and log the user out.
        axios
            .get("http://localhost:8080/api/logout", {
                withCredentials: true, // The browser will send the JWT token cookie automatically, if it has it!
            })
            .then((res) => {
                console.log("Logout response:", res);
            })
            .catch((err) => {
                console.log(err);
                alert("Could not logout!");
            })
            .finally(() => {
                setLoggedUser(null);
                redirect("/");
            });
    };

    return (
        <div className="container py-5">
            <div className="row g-2">
                <div className="col-md-5">
                    <div className="card shadow-sm">
                        <div className="card-header bg-primary text-white">
                            <h5 className="mb-0">Users:</h5>
                        </div>
                        <ul className="list-group list-group-flush">
                            {usersList.map((user, index) => (
                                <li
                                    key={index}
                                    className="list-group-item d-flex justify-content-between align-items-center"
                                >
                                    <p className="mb-1 small">
                                        User ID: {user.userID}
                                    </p>
                                    <p className="mb-1 small">
                                        Username: {user.username}
                                    </p>
                                    <span className="badge bg-secondary rounded-pill">
                                        Active
                                    </span>
                                </li>
                            ))}
                        </ul>
                    </div>
                </div>

                <div className="col-md-4">
                    <div className="card shadow-sm h-100">
                        <div className="card-body d-flex flex-column justify-content-center align-items-center">
                            <h5 className="card-title mb-3">
                                Fetch Random Image
                            </h5>
                            <p className="card-text text-muted">
                                Click below to retrieve a sample image
                            </p>

                            {renderImage ? (
                                <img
                                    className="img-fluid rounded-3 mb-4"
                                    src={renderImage}
                                    alt="Fetched Random cat image from server"
                                    style={{
                                        objectFit: "cover",
                                        maxHeight: "200px",
                                    }}
                                />
                            ) : null}
                            <button
                                type="button"
                                className="btn btn-outline-success mt-auto"
                                onClick={getImage}
                            >
                                {renderImage ? "Fetch New Image" : "Get Image"}
                            </button>
                        </div>
                    </div>
                </div>

                <div className="col-md-3">
                    <div className="card shadow-sm h-100">
                        <div className="card-body d-flex flex-column justify-content-center align-items-center">
                            <h5 className="card-title">Account actions</h5>
                            <p className="card-text ">
                                Logged in as : {loggedUser?.username || "Guest"}
                            </p>
                            <button
                                type="button"
                                className="btn btn-danger mt-3"
                                onClick={handleLogout}
                            >
                                Logout
                            </button>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
};

export default Dashboard;
