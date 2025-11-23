import { BrowserRouter, useNavigate, Switch, Route, Link, Routes } from "react-router-dom";
import './styles/global.css'
import Landing_page from './components/regLogin/Landing_page'
import { UserProvider } from "./context/UserContext";
import Dashboard from "./components/Dashboard";

const App = () => {
    return (
        <UserProvider>
            <BrowserRouter>
                <Routes>
                    <Route path="/" element={<Landing_page />}></Route>
                    <Route path="/Dashboard" element={<Dashboard />}></Route>
                </Routes>
            </BrowserRouter>
        </UserProvider>
    );
}

export default App;