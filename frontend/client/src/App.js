import { BrowserRouter, Route, Routes } from "react-router-dom";
import './styles/global.css'
import LandingPage from './components/regLogin/LandingPage'
import { UserProvider } from "./context/UserContext";
import Dashboard from "./components/Dashboard";

const App = () => {
    return (
        <UserProvider>
            <BrowserRouter>
                <Routes>
                    <Route path="/" element={<LandingPage />}></Route>
                    <Route path="/Dashboard" element={<Dashboard />}></Route>
                </Routes>
            </BrowserRouter>
        </UserProvider>
    );
}

export default App;