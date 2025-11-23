import React, { useState, useEffect, useContext } from "react";
import "../../styles/regLogin.css";

import axios from "axios";
import { UserContext } from "../../context/UserContext";
import { useNavigate } from "react-router-dom";

const onlyLettersRegex = new RegExp(/^[A-Za-z]+$/);

const RegisterLogin = () => {
    const { loggedUser, setLoggedUser } = useContext(UserContext);
    const redirect = useNavigate();
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [formErrors, setFormErrors] = useState({
        usernameError: "",
        passwordError: "",
    });

    // Watch for changes of loggedUser variable, so we only redirect when the update has taken place and is being rendered.
    useEffect(() => {
        if (loggedUser) {
            console.log(
                "Context updated! Redirecting... loggedUser:",
                loggedUser
            );
            redirect("/dashboard");
        }
    }, [loggedUser, redirect]);

    const submitForm = (data, url, purpose = "Login") => {
        axios
            .post(`http://localhost:8080/${url}`, data, {
                withCredentials: true,
            })
            .then((res) => {
                console.log(`${purpose} successful`, res.data);
                const userData = {
                    userID: res.data.userID,
                    username: res.data.username,
                };
                setLoggedUser(userData); // this triggers the re-render and useEffect above will run
            })
            .catch((err) => {
                console.log(err.response.data["Error"]);
                return alert(`Error: ${err.response.data["Error"]}`);
            });
    };

    const validateUsername = (text) => {
        let error = "";
        if (text.length < 3 || text.length > 32 || text.length === 0) {
            error = "Username must be between 3 and 32 characters!";
        } else if (!onlyLettersRegex.test(text)) {
            error = "Username must only contain letters!";
        }
        return error;
    };

    const handleUsernameChange = (e) => {
        const newUsername = e.target.value;
        setUsername(newUsername);
        const error = validateUsername(newUsername);
        setFormErrors((prevErrors) => ({
            ...prevErrors,
            usernameError: error,
        }));
    };

    const validatePassword = (e) => {
        const newPassword = e.target.value;
        setPassword(newPassword);
        let error = "";
        if (
            newPassword.length < 5 ||
            newPassword.length > 32 ||
            newPassword.length === 0
        ) {
            error = "Password must be between 5 and 32 characters!";
        }
        setFormErrors((prevErrors) => ({
            ...prevErrors,
            passwordError: error,
        }));
    };

    const register = (e) => {
        e.preventDefault();
        if (
            formErrors["passwordError"].length !== 0 ||
            formErrors["usernameError"].length !== 0
        ) {
            console.log("Errors exist");
            return;
        }

        const params = new URLSearchParams();
        params.append("username", username);
        params.append("password", password);

        submitForm(params, "api/register", "Register");
    };

    const login = (e) => {
        e.preventDefault();

        const params = new URLSearchParams();
        params.append("username", username);
        params.append("password", password);

        submitForm(params, "api/login", "Login");
    };

    return (
        <div className="container">
            <p>Enter info to register or login!</p>
            <form>
                <input
                    type="text"
                    placeholder="Username"
                    name="username"
                    onChange={handleUsernameChange}
                    value={username}
                />
                {formErrors["usernameError"] ? (
                    <div className={"reg__err"}>
                        <p>{formErrors["usernameError"]}</p>
                    </div>
                ) : null}

                <input
                    type="password"
                    placeholder="Password"
                    name="password"
                    onChange={validatePassword}
                    value={password}
                />

                {formErrors["passwordError"] ? (
                    <div className={"reg__err"}>
                        <p>{formErrors["passwordError"]}</p>
                    </div>
                ) : null}
            </form>
            {formErrors["passwordError"] === "" &&
            formErrors["usernameError"] === "" &&
            username !== "" &&
            password !== "" ? (
                <div className="btns">
                    <button className="regLoginBtn" onClick={register}>
                        Register
                    </button>
                    <button className="regLoginBtn" onClick={login}>
                        Login
                    </button>
                </div>
            ) : null}
        </div>
    );
};

export default RegisterLogin;
