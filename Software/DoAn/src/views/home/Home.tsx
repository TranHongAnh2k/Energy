import {
  BarElement, CategoryScale, Chart as ChartJS, Legend, LinearScale, Title,
  Tooltip
} from 'chart.js';
import classnames from 'classnames';
import React from 'react';
import { Bar } from 'react-chartjs-2';
import { Card, CardBody, CardTitle, Col, Nav, NavItem, NavLink, Row, TabContent, TabPane } from 'reactstrap';
import getInstanceFirebase from '../../firebase/firebase';
import CURRENT from './../../assets/img/icon/current.png';
import ENERGY from './../../assets/img/icon/energy.png';
import VOLTAGE from './../../assets/img/icon/voltage.png';
interface Props { }
interface State {
  activeTab: string,
  listMacAddress: string[],
  data: Data[],
  listCurrent: number[]
  listEnergy: number[],
  listVoltage: number[]
}
interface Data {
  voltage: string
  current: string
  energy: string
  month : string
}
const firebase = getInstanceFirebase();

ChartJS.register(
  CategoryScale,
  LinearScale,
  BarElement,
  Title,
  Tooltip,
  Legend
);
const options = {
  responsive: true,
  plugins: {
    legend: {
      position: 'top' as const,
    },
    title: {
      display: true,
      text: 'Biểu đồ đánh giá điện năng tiêu thụ trong tòa nhà',
    },
  },
};
export default class Home extends React.PureComponent<Props, State> {
  constructor(props: Props) {
    super(props);
    this.toggle = this.toggle.bind(this);
    this.state = {
      activeTab: '34AB9524532F',
      listMacAddress: [],
      data: [],
      listCurrent: [],
      listEnergy: [],
      listVoltage: [],
    };
    this.callApi()
  }
  componentDidMount() {
    this.getMacAddress()

  }

  callApi = () => {
    setTimeout(() => {
      this.getData()
    }, 60000)
  }
  getData = async () => {
    try {

      const { activeTab } = this.state
      const res: Data[] = await firebase.getData(activeTab.toString())
      const data = res.sort((a: Data, b: Data) => Number(a.month) - Number(b.month));
      this.setState({
        data: data,
        listCurrent: data.map((value: Data) => Number(value.current)),
        listEnergy: data.map((value: Data) => Number(value.energy)),
        listVoltage: data.map((value: Data) => Number(value.voltage)),
      })
    } catch (error) {
      console.log(error);

    }
  }
  getMacAddress = async () => {
    try {
      const res: any[] = await firebase.getMacAddress()
      this.setState({ listMacAddress: res, activeTab: res[0].value }, this.getData)
    } catch (error) {
      console.log(error);
    }
  }
  toggle(tab: string) {
    if (this.state.activeTab !== tab) {
      this.setState({
        activeTab: tab
      }, this.getData);
    }
  }
  _renderNavBar = (mac: string, index: number) => {
    const { activeTab } = this.state
    return (
      <>

        <NavItem>
          <NavLink
            className={classnames({ active: activeTab === mac })}
            onClick={() => { this.toggle(mac) }}
          >
            Thiết bị {index + 1}
          </NavLink>
        </NavItem>
      </>
    )
  }
  _renderTabContent = (mac: string) => {
    const {listEnergy,listVoltage,listCurrent } = this.state
    const labels = ['Tháng 1', 'Tháng 2','Tháng 3','Tháng 4','Tháng 5','Tháng 6','Tháng 7','Tháng 8','Tháng 9','Tháng 10','Tháng 11','Tháng 12',]
    const date = new Date()
    const month = date.getMonth() 

    const datas = {
      labels,
      datasets: [
        {
          label: 'Công suất tiêu thụ',
          data: listEnergy,
          borderColor: 'blue',
          backgroundColor: 'blue',
        },
      ],
    }
    return (
      <>
        <TabPane tabId={mac}>
          <Row>
            <Col lg='4' md='4' sm='12'>
              <Card className={'card-stats'} id={'temperature-popover'}>
                <CardBody>
                  <Row>
                    <Col md='4' xs='5'>
                      <div className='icon-big text-center icon-warning icon-card'>
                        <img src={VOLTAGE} alt='' />
                      </div>
                    </Col>
                    <Col md='8' xs='7'>
                      <div className='numbers'>
                        <CardTitle tag='p'>Điện áp</CardTitle>
                        <p className='card-category'>{
                        listVoltage[month]? listVoltage[month] : 0} V</p>
                      </div>
                    </Col>
                  </Row>
                </CardBody>
              </Card>
            </Col>
            <Col lg='4' md='4' sm='12'>
              <Card className='card-stats'>
                <CardBody>
                  <Row>
                    <Col md='4' xs='5'>
                      <div className='icon-big text-center icon-warning icon-card'>
                        <img src={CURRENT} alt='' />
                      </div>
                    </Col>
                    <Col md='8' xs='7'>
                      <div className='numbers'>
                        <CardTitle tag='p'>Dòng điện</CardTitle>
                        <p className='card-category'>{listCurrent[month] ? listCurrent[month] : 0} A</p>
                      </div>
                    </Col>
                  </Row>
                </CardBody>
              </Card>
            </Col>
            <Col lg='4' md='4' sm='12'>
              <Card className='card-stats'>
                <CardBody>
                  <Row>
                    <Col md='4' xs='5'>
                      <div className='icon-big text-center icon-warning icon-card'>
                        <img src={ENERGY} alt='' />
                      </div>
                    </Col>
                    <Col md='8' xs='7'>
                      <div className='numbers'>
                        <CardTitle tag='p'>Điện áp tiêu thụ</CardTitle>
                        <p className='card-category'>{listEnergy[month] ? listEnergy[month] : 0} kWh</p>
                      </div>
                    </Col>
                  </Row>
                </CardBody>
              </Card>
            </Col>
            <Col xl='2' />
            <Col xl='8'>
              <Bar options={options} data={datas} />
            </Col>
          </Row>
        </TabPane>
      </>
    )
  }
  render() {
    const { listMacAddress, activeTab, data } = this.state
    return (
      <div className='m-3'>
        <h2 className='d-flex justify-content-center pt-4 '>Mạng thu thập dữ liệu đánh giá mức độ tiêu thụ điện năng của tòa nhà </h2>
        <Nav tabs>
          {listMacAddress.map((mac: any, index: number) => this._renderNavBar(mac.value, index))}
        </Nav>
        {
          data.length > 0 &&
          <TabContent activeTab={activeTab}>
            {listMacAddress.map((mac: any, index: number) => this._renderTabContent(mac.value))}
          </TabContent>
        }
      </div>
    );
  }
}