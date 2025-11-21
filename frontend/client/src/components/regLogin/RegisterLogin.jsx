import React, { useState, useEffect, useRef, useContext } from "react";
import "../../styles/regLogin.css";

import axios from "axios";
import { UserContext } from "../../context/UserContext";
import { useNavigate } from "react-router-dom";

const onlyLettersRegex = new RegExp(/^[A-Za-z]+$/);

const RegisterLogin = ({ vibrateErr, changeErrVibrate }) => {
    const { loggedUser, setLoggedUser } = useContext(UserContext);
    const redirect = useNavigate();
    const [username, setUsername] = useState("");
    const [password, setPassword] = useState("");
    const [formErrors, setFormErrors] = useState({
        usernameError: "",
        passwordError: "",
    });

    const regUser = (e) => {
        // e.preventDefault();
        // if (currInput === "email") {
        //     setFormErrors({ checkingEmail: "checking if email is available" });
        //     checkTakenEmail(newUser.email);
        //     return;
        // } else if (Object.keys(formErrors).length > 0) {
        //     changeErrVibrate();
        //     return;
        // } else if (
        //     inputsInOrder.indexOf(currInput) !==
        //     inputsInOrder.length - 1
        // ) {
        //     setCurrInput(inputsInOrder[inputsInOrder.indexOf(currInput) + 1]);
        //     return;
        // }
        // console.log("all input entered");
        // const formData = new FormData();
        // formData.append("first_name", newUser.firstName);
        // formData.append("last_name", newUser.lastName);
        // formData.append("age", newUser.age);
        // formData.append("email", newUser.email);
        // formData.append("password", newUser.password);
        // formData.append("confirmPassword", newUser.confirmPassword);
        // formData.append("pfp", newUser.profilePic);
        // submitForm(formData, "api/users/register/");
    };

    const submitForm = (data, url) => {
        axios
            .post(`http://localhost:8000/${url}`, data, {
                withCredentials: true,
            })
            .then((res) => {
                console.log(res);
                if (res.data["errors"] === true) {
                    if (res.data["loginFail"]) {
                        changeErrVibrate();
                        return setFormErrors({ loginFail: res.data["msg"] });
                    } else {
                        return alert(
                            "error please refresh page",
                            res.data["body"]
                        );
                    }
                }
                console.log("successfully login or registered user");
                setLoggedUser(res.data.body.user);
                redirect("/Dashboard");
            })
            .catch((err) => {
                console.log(err);
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
        console.log(username, password)
    };
    const login = (e) => {
        e.preventDefault();
        console.log(username, password)
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
                    <div className={`reg__err ${vibrateErr}`}>
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
                    <div className={`reg__err ${vibrateErr}`}>
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
